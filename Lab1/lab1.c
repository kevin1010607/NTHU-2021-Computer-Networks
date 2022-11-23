#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<arpa/inet.h>
#include<netdb.h>

int sockfd = 0;
struct sockaddr_in info;
char url[100], hostname[100], request[100], response[50000], buffer[50000];
char **hyperlink;
int len = 0, count = 0;

char** getHyperlink(char* str, int size, int* cnt){
    char **res = (char**)malloc(100*sizeof(char*));
    char pattern[] = "<a href=\"";
    for(int i = 0; i < size; i++){
        if(!strncmp(str+i, pattern, 9)){
            i += 9;
            int length = 0;
            while(str[i+length] != '\"') length++;
            res[*cnt] = (char*)malloc((length+1)*sizeof(char));
            strncpy(res[*cnt], str+i, length);
            res[*cnt][length] = '\0';
            (*cnt)++;
            i += length;
        }
    }
    return res;
}

int main(int argc, char *argv[]){
    // url, hostname
    printf("Enter the hostname: ");
    scanf("%s", url);
    int idx = 0;
    while(url[idx]!='\0' && url[idx]!='/'){
        hostname[idx] = url[idx];
        idx++;
    }
    hostname[idx] = '\0';

    // socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("\n Socket Creation Failed. \n");
        return -1;
    }

    // sockaddr
    bzero(&info, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = htons(80);
    struct hostent *h = gethostbyname(hostname);
    memcpy(&info.sin_addr.s_addr, h->h_addr_list[0], h->h_length);

    // connect
    if(connect(sockfd, (struct sockaddr*)&info, sizeof(info)) == -1){
        printf("\n Connection Failed. \n");
        return -1;
    }

    // request
    strcat(request, "GET http://");
    strcat(request, url);
    strcat(request, " HTTP/2.0\r\n\r\n");

    // send
    printf("socket: Start send HTTP request\n");
    if(send(sockfd, request, sizeof(request), 0) == -1){
        printf("\n Send Failed. \n");
        return -1;
    }

    // recv
    printf("socket: Start read the response\n");
    int num;
    while((num = recv(sockfd, buffer, sizeof(buffer), 0)) > 0){
        strncat(response, buffer, num);
        len += num;
    }
    if(num == -1){
        printf("\n Recv Failed. \n");
        return -1;
    }
    printf("socket: Finish read to buffer\n");

    // just for debugging
    // printf("%s", response);
    // printf("%d %d\n", (int)strlen(response), len);

    // get, output hyperlinks and free space
    hyperlink = getHyperlink(response, len, &count);
    printf("======== Hyperlinks ========\n");
    for(int i = 0; i < count; i++){
        printf("%s\n", hyperlink[i]);
        free(hyperlink[i]);
    }
    free(hyperlink);
    printf("%d\n", count);

    return 0;
}