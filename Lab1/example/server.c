#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h> 
#include <sys/wait.h>
#include <poll.h>

//==============
// packet header 
//==============
typedef struct header{
	unsigned int seq_num;
	unsigned int ack_num;
	unsigned char isLast;
}Header;

//==================
// Udp packet & data 
//==================
typedef struct udp_pkt{
	Header header;
	char data[1024];
}Udp_pkt;

// declaration
int sockfd;
struct sockaddr_in info, client_info;
Udp_pkt snd_pkt,rcv_pkt;
socklen_t len;

//=====================
// not work so not used
//=====================
int	sendFile(FILE *fd)
{	
	int filesize=ftell(fd);
	int have_send=0;
	int recieve=0;
	
	int timeout=0;
	while(filesize>0)
	{
		fseek(fd,have_send,SEEK_SET);
		fread(snd_pkt.data,sizeof(unsigned char),1024,fd);

		//===================================================
		// uncomment to comfirm we have send packet not empty
		//===================================================
		/*
		for(int i=0;i<1024;i++)
			printf("%u ",snd_pkt.data[i]);
		cout<<endl;
		*/
		
		//==========================
		// send vedio data to client
		//==========================
		if ((sendto(sockfd, &snd_pkt, sizeof(snd_pkt), 0,(struct sockaddr *)&client_info, len)) == -1) 
		{
			perror("error");
			return -1;
		}
		printf("\tSend 1036 byte\n"); // (12+1024) header+data

		//=====================================
		// start polling for receive client ack
		//=====================================	
		struct pollfd pfd;
		int ret;

		pfd.fd = sockfd; // your socket handler 
		pfd.events = POLLIN;
		ret = poll(&pfd, 1, 100); // 0.1 second for timeout
		switch (ret) {
			case -1:
				// Error
				break;
			case 0:
				// Timeout 
				timeout=1;
				break;
			default:
				recvfrom(sockfd, &rcv_pkt, sizeof(rcv_pkt), 0,(struct sockaddr *)&client_info, (socklen_t *)&len); // get your data
				recieve++;
				printf("\tReceive a packet (seq_num = %u, ack_num = %u)\n",rcv_pkt.header.seq_num, rcv_pkt.header.ack_num);
				break;
		}
		if(timeout==1)
		{
			//==========================
			// if timeout, resend packet
			//==========================	
			timeout=0;
			printf("\tTimeout! Resend packet!\n");
			continue;
		}
		snd_pkt.header.seq_num ++;
		snd_pkt.header.ack_num ++;  
		have_send += 1024;
		filesize -= 1024;
		
		//========================
		// the last part of packet
		//========================
		if(filesize<=1024)
		{
			snd_pkt.header.isLast=1;
			if ((sendto(sockfd, &snd_pkt, sizeof(snd_pkt), 0,(struct sockaddr*)&client_info, len)) == -1) 
			{
			
				perror("error");
				return -1;
					
			}
			break;
		}
		printf("send file successfully\n");
	}
	
	fclose(fd);
    return 0;
}

int main(int argc, char *argv[])
{
	//===========================
	// argv[1] is for server port
	//===========================
	//int sockfd = 0;
	sockfd = socket(AF_INET , SOCK_DGRAM , 0);

	if (sockfd == -1)
	{
		printf("Fail to create a socket.");
	}
	//=======================
	// enter server info
	// IP address = 127.0.0.1
	//=======================
	//struct sockaddr_in info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;
	int port=atoi(argv[1]);
	info.sin_addr.s_addr = INADDR_ANY ;
	info.sin_port = htons(port);
	//printf("server %s : %d\n", inet_ntoa(info.sin_addr), htons(info.sin_port));

	//================
	// bind the socket
	//================	
	if(bind(sockfd, (struct sockaddr *)&info, sizeof(info)) == -1)
	{
		perror("server_sockfd bind failed: ");
		return 0;
	}

	//====================================
	// create send packet & receive packet
	//====================================
	//Udp_pkt snd_pkt,rcv_pkt;
	memset(rcv_pkt.data, '\0', sizeof(rcv_pkt.data));

	//====================
	// use for client info
	//====================
	//struct sockaddr_in client_info;
	bzero(&client_info,sizeof(client_info));
	client_info.sin_family = AF_INET;
	len = sizeof(client_info);

	printf("====Parameter====\n");
	printf("Server's IP is 127.0.0.1\n");
	printf("Server is listening on port %d\n",port);
	printf("==============\n");

	while(1)
	{
		//=========================
		// initialization parameter
		//=========================
		snd_pkt.header.seq_num = 1;
		snd_pkt.header.ack_num = 1;
		snd_pkt.header.isLast=0;
		FILE *fd;
		
		printf("server waiting.... \n");
		char *str;
		while ((recvfrom(sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&client_info, (socklen_t *)&len)) != -1)
		{
			if(rcv_pkt.header.isLast==1)
				break;
		}  
		printf("process command.... \n");
		str = strtok(rcv_pkt.data, " ");
		
		// ==============================================================
        // command "download filename": download the file from the server
		// check if filename is exist
        // ==============================================================
		if(strcmp(str, "download") == 0) 
		{
            str = strtok(NULL, " \n");
			printf("filename is %s\n",str);
			// ==================
			// if file not exists 
			// ==================
			if((fd = fopen(str, "rb")) == NULL) 
			{    
				// ======================================
				// send FILE_NOT_EXISTS msg to the client
                // ======================================
				printf("FILE_NOT_EXISTS\n");
                strcpy(snd_pkt.data, "FILE_NOT_EXISTS");
				int numbytes;
                if ((numbytes = sendto(sockfd, &snd_pkt, sizeof(snd_pkt), 0,(struct sockaddr *)&client_info, len)) == -1) 
				{
					printf("sendto error\n");
					return 0;
				}
				printf("server: sent %d bytes to %s\n", numbytes,inet_ntoa(client_info.sin_addr));
            } 
			// =================
			// else, file exists 
			// =================
			else 
			{
				fseek(fd,0,SEEK_END);
                printf("FILE_EXISTS\n");
				strcpy(snd_pkt.data, "FILE_EXISTS");
				
				
				// =================================
				// send FILE_EXIST msg to the client
				// =================================
				int numbytes;
				if ((numbytes = sendto(sockfd, &snd_pkt, sizeof(snd_pkt), 0,(struct sockaddr *)&client_info, len)) == -1) 
				{
					printf("sendto error\n");
					return 0;
				}
				printf("server: sent %d bytes to %s\n", numbytes,inet_ntoa(client_info.sin_addr));

				// =========================================================================
				// sleep 1 seconds before transmitting data to make sure the client is ready
				// =========================================================================
				sleep(1);
				printf("trasmitting...\n");

				// ====================================
				// start to send the file to the client
				// ====================================

				sendFile(fd);
				
            }
		}
		else 
		{
            printf("Illegal request!\n");   
        }
	}
}
