#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h> 

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
int sockfd = 0;
Udp_pkt snd_pkt,rcv_pkt;
struct sockaddr_in info, client_info;
socklen_t len;

//=====================
// simulate packet loss
//=====================
int isSuc(double prob)
{
    double thres = prob * RAND_MAX;
    
    if(prob >= 1)
        return 1;
    printf("\tOops! Packet loss!\n");
    return (rand() <= thres);    
}

//=====================
// not work so not used
//=====================
int recvFile(FILE *fd)
{
	printf("FILE_EXISTS\n");

	char* str;
	char fileName[30];
	time_t t1, t2;

	//==================================================================
	// split the command into "download" & "filename", just get filename
	//==================================================================
	str = strtok(snd_pkt.data, " \n");	
	str = strtok(NULL, " \n");			
	
	sprintf(fileName, "download_");
	strcat(fileName, str);
	
	//FILE *fd;
	fd = fopen(fileName, "wb");
	
	printf("Receiving...\n");
	char buffer[123431];
	int index=0;
	int receive_packet=0;
	memset(snd_pkt.data, '\0', sizeof(snd_pkt.data));
	while(1) 
	{
		//=======================
		// send command to server
		//=======================
		int numbytes;
		while ((numbytes = recvfrom(sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&info, (socklen_t *)&len)) != -1)
		{
			//===================================================
			// if receive the last packet, write buffer into file 
			//===================================================
			if(rcv_pkt.header.isLast==1)
			{
				fseek(fd, 0, SEEK_SET);
				fwrite(buffer, sizeof(buffer), 1, fd);
				fclose(fd);
				printf("client received finished\n");
				return 0;		
			}

			//=======================
			// simulation packet loss
			//=======================
			if(isSuc(0.5))
			{
				break;
			}
				
			//==============================================
			// actually receive packer and write into buffer
			//==============================================
			receive_packet++;
			printf("\tReceive a packet (seq_num = %u, ack_num = %u)\n", rcv_pkt.header.seq_num, rcv_pkt.header.ack_num);
			for(int i=0;i<1024;i++)
			{
				buffer[index]=rcv_pkt.data[i];
				index++;
			}	

			//======================================================
			// uncomment to comfirm we have receive packet not empty
			//======================================================
			/* 
			for(int i=0;i<index;i++)
				printf("%u ",buffer[i]);
			cout<<endl;
			*/

			//====================
			// reply ack to server
			//====================
			snd_pkt.header.ack_num=rcv_pkt.header.ack_num+1+1;
			if ((sendto(sockfd, &snd_pkt, sizeof(snd_pkt), 0,(struct sockaddr *)&info, len)) == -1) 
			{
				perror("error");
				return -1;
			}
			
			memset(rcv_pkt.data, '\0', sizeof(rcv_pkt.data));
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	//==============
	// create socket
	//==============
	//int sockfd = 0;
	sockfd = socket(AF_INET , SOCK_DGRAM , 0);
	if (sockfd == -1)
	{
		printf("Fail to create a socket.");
	}

	//==================
	// enter server info
	//==================
	//struct sockaddr_in info;
	bzero(&info,sizeof(info));
	info.sin_family = AF_INET;

	char* server_ip=(char *)malloc(sizeof(char)*30);
	int server_port;

	printf("give me an IP to send: ");
	scanf("%s",server_ip);
	printf("server's's port? ");
	scanf("%d",&server_port);

	//==================================
	// just test how to convert the type
	//==================================	
	info.sin_addr.s_addr = inet_addr(server_ip);
	info.sin_port = htons(server_port);

	server_port=htons(info.sin_port);
	server_ip=inet_ntoa(info.sin_addr);
	//printf("server %s : %d\n", server_ip, server_port);


	//====================================
	// create send packet & receive packet
	//====================================
	memset(snd_pkt.data, '\0', sizeof(snd_pkt.data));
	len = sizeof(info);

	printf("Waiting for a commands...\n");
	getchar();
	while(fgets(snd_pkt.data, 30, stdin)) 
	{
        
        // ================================
        // command "exit": close the client
        // ================================
		if(strncmp(snd_pkt.data, "exit", 4) == 0) 
		{
            break;
        // ==============================================================
        // command "download filename": download the file from the server
        // ==============================================================
        }
		else if(strncmp(snd_pkt.data, "download", 8) == 0) 
		{
			snd_pkt.header.seq_num = 0;
			snd_pkt.header.ack_num = 0;
			snd_pkt.header.isLast = 1;
			int numbytes;
			FILE* fd;
			//========================
			// send filename to server
			//========================
			if ((numbytes = sendto(sockfd, &snd_pkt, sizeof(snd_pkt), 0,(struct sockaddr *)&info, len)) == -1) 
			{
				perror("error");
				return 0;
			}
			printf("client: sent %d bytes to %s\n", numbytes,inet_ntoa(info.sin_addr));
			//=========================================
			// get server response if file exist or not
			//=========================================
			if ((numbytes = recvfrom(sockfd, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&info, (socklen_t *)&len)) == -1)
			{
				printf("recvfrom error\n");
				return 0;
			}	
			printf("client: receive %d bytes from %s\n", numbytes,inet_ntoa(info.sin_addr));
			//printf("%s", buf);
			
			//====================
			// file does not exist
			//====================
			if(strcmp(rcv_pkt.data,"FILE_NOT_EXISTS") == 0) 
			{
				printf("FILE_NOT_EXISTS\n");
			} 
			//====================================================
			// file exists, maybe can modify into function version
			//====================================================
			else if(strcmp(rcv_pkt.data,"FILE_EXISTS") == 0) 
			{
				recvFile(fd);
			}	
			
		}
		else 
		{
			printf("Illegal command\n");	
		}
        
        printf("Waiting for a commands...\n");
    }

}


