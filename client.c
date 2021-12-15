#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

#define SOCKET int

#define SERV_PORT 8080
#define MAXDATA   1024
#define MAXNAME 1024

int socket_fd;
char account[30];
char passwd[30];

void* recv_thread(void* p){
	char *ptr,*qtr;
	while(1){
		char buf[100] = {};
		if (recv(socket_fd, buf, sizeof(buf), 0) <= 0){
			return;
		}
		if (strcmp(buf, "acc_pwd") == 0){
			send(socket_fd, account, strlen(account), 0);
			break;
		}
		else{
			printf("%s\n",buf);
		}
	}
}

int main(int argc, char *argv[])
{
    int length;
    char buf[BUFSIZ];
    struct hostent *hp;

    struct sockaddr_in myaddr;
    struct sockaddr_in servaddr; 

    if (argc < 2) {
      fprintf (stderr, "Usage: %s host_name(IP address)\n", argv[0]);
      exit(2);
    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror ("socket failed!");
      exit(1);
    }

    bzero((char *)&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);

    if (bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) <0){
        perror("bind failed!");
        exit(1);
    }

    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);


    hp = gethostbyname(argv[1]);
    if (hp == 0) {
        fprintf(stderr, "could not obtain address of %s\n", argv[2]);
        return (-1);
    }

    bcopy(hp->h_addr_list[0], (caddr_t)&servaddr.sin_addr, hp->h_length);

    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect failed!");
        exit(1);
    }
 
    printf("account : ");
    scanf("%s", account);
    printf("password : ");
    scanf("%s", passwd);
    strcat(account, ":");
    strcat(account, passwd);
    strcat(account, "\0");

    while(1){
        char test_acc[50];
		recv(socket_fd, test_acc, sizeof(test_acc), 0);
		if (strcmp(test_acc,"acc_pwd") == 0){
			send(socket_fd, account, strlen(account), 0);
			break;
		}
        
	}
    pthread_t id;
	pthread_create(&id,0,recv_thread,0);

    //start
    while(1){
        char buf[100] = {};

		fgets(buf, sizeof(buf), stdin);

        if(buf[0]=='O' && buf[1]=='t'){
            //printf("send\n");
		    send(socket_fd, "Other users", strlen("Other users"), 0);
		}
        else if(buf[0] == '*' || buf[0] == '#' || buf[0] == '$')
		{
            char send_msg[100] = {};
			sprintf(send_msg,"%s",buf);
			send(socket_fd, send_msg, strlen(send_msg), 0);
		}
        else if(buf[0] == 'y' && buf[1] == 'e' && buf[2] == 's'){
            printf("Connect successful!\n");

            char send_msg[100] = {};
            sprintf(send_msg,"YES");
			send(socket_fd, send_msg,strlen(send_msg), 0);
			printf("Start!\n");
        }
		else if(buf[0] == 'q' && buf[1] == 'u' && buf[2] == 'i' && buf[3] == 't'){
            char send_msg[100] = {};
			sprintf(send_msg,"%s",buf);
			send(socket_fd, send_msg, strlen(send_msg), 0);
        }
    }


    close (socket_fd);
}