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
#define MAXNAME 1024


extern int errno;
int fd_user_list[20];

int game_list[20][2];

struct{
	char user_name[100];
}now_account[100];

char game_board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int game_time;

int judge()
{
	
	if(game_board[0] == 'O' && game_board[1] == 'O' && game_board[2] == 'O'){
		return 1;
	}
	if(game_board[3] == 'O' && game_board[4] == 'O' && game_board[5] == 'O'){
		return 1;
	}
	if(game_board[6] == 'O' && game_board[7] == 'O' && game_board[8] == 'O'){
		return 1;
	}
	if(game_board[0] == 'O' && game_board[3] == 'O' && game_board[6] == 'O'){
		return 1;
	}
	if(game_board[1] == 'O' && game_board[4] == 'O' && game_board[7] == 'O'){
		return 1;
	}
	if(game_board[2] == 'O' && game_board[5] == 'O' && game_board[8] == 'O'){
		return 1;
	}
	if(game_board[0] == 'O' && game_board[4] == 'O' && game_board[8] == 'O'){
		return 1;
	}
	if(game_board[2] == 'O' && game_board[4] == 'O' && game_board[6] == 'O'){
		return 1;
	}

	if(game_board[0] == 'X' && game_board[1] == 'X' && game_board[2] == 'X'){
		return 2;
	}
	if(game_board[3] == 'X' && game_board[4] == 'X' && game_board[5] == 'X'){
		return 2;
	}
	if(game_board[6] == 'X' && game_board[7] == 'X' && game_board[8] == 'X'){
		return 2;
	}
	if(game_board[0] == 'X' && game_board[3] == 'X' && game_board[6] == 'X'){
		return 2;
	}
	if(game_board[1] == 'X' && game_board[4] == 'X' && game_board[7] == 'X'){
		return 2;
	}
	if(game_board[2] == 'X' && game_board[5] == 'X' && game_board[8] == 'X'){
		return 2;
	}
	if(game_board[0] == 'X' && game_board[4] == 'X' && game_board[8] == 'X'){
		return 2;
	}
	if(game_board[2] == 'X' && game_board[4] == 'X' && game_board[6] == 'X'){
		return 2;
	}
	return 0;
}

int authentication(char buf[])
{
    FILE *fp;
	char tmp1[100];
	fp=fopen("passwd","r");

	while(fscanf(fp,"%s",tmp1)!=EOF){
		if(strcmp(tmp1,buf)==0)
			return 1;
	}
	return 0;
}

void* service_thread(void* data){
	int fd_user = *(int*)data;
	char *ptr,tmp[100];
	printf("pthread = %d\n",fd_user);

	char buf[100] = {};
	//LOGIN
	while(1){
		send(fd_user, "acc_pwd", strlen("acc_pwd"),0);
		recv(fd_user, buf,sizeof(buf), 0);
		printf("buf=%s\n",buf);

		for(int i = 0; i < 100; i++){
			if(buf[i] == '\0'){
				break;
			}
			if(buf[i] == ':'){
				buf[i] = '\0';
				strcpy(now_account[fd_user].user_name, buf);
				buf[i] = ':';
				break;
			}
		}

        printf("account=%s\n",now_account[fd_user].user_name);

        int a = authentication(buf);
		//printf("%d\n", a);
		if(a == 1){
			printf("New account : %s\n",  now_account[fd_user].user_name);
			break;
		}
		if(a == 0){
            printf("fail\n");
            pthread_kill(fd_user, SIGALRM);
        }
	}
	while(1){
		char buf2[100] = {};

		recv(fd_user, buf2, sizeof(buf2), 0);
		//printf("buf2=%s\n",buf2);

		if(strcmp(buf2, "Other users") == 0){
			//printf("%d ls.\n", fd_user);
			send(fd_user, "[user list]\n", strlen("[user list]\n"), 0);

			for (int i = 0;i < 100;i++){
				if (fd_user_list[i] != 0){
					char buf_send[100] = {};

					if(fd_user_list[i]!=fd_user){
						sprintf(buf_send, "user: %s fd:%d\n" ,now_account[fd_user_list[i]].user_name, fd_user_list[i]);
						printf("%s", buf_send);
						send(fd_user, buf_send, strlen(buf_send), 0);
					}
				}
			}
		}
		else if(buf2[0]=='*'){

			int fd2 = atoi(&buf2[1]);

			game_list[fd_user][0] = fd_user;
			game_list[fd_user][1] = fd2;

			char buf_send[300];
			sprintf(buf_send, "CONNECT %s %d\n" ,now_account[fd_user].user_name, fd_user);

			//printf("%s\n", buf_send);
			send(fd2, buf_send, strlen(buf_send), 0);

			send(fd_user,"connecting\n\0",strlen("connecting\n\0"),0);

			char buf_send2[300];
			sprintf(buf_send2, "connect with %s?(yes or no)\n" ,now_account[fd_user].user_name);
			send(fd2, buf_send2, strlen(buf_send2), 0);
			//printf("%s\n", buf_send2);
		}
		else if(strncmp(buf2,"YES",3)==0){
			int fd2 = fd_user;
			int fd1;
			for(int i = 0; i < 20; i++){
				if(game_list[i][1] == fd2){
					fd1 = game_list[i][0];
				}
			}
			game_list[fd2][0] = fd2;
			game_list[fd2][1] = fd1;

			char buf_send[300];
			sprintf(buf_send, "%s agree\nGame Start!\n" ,now_account[fd_user].user_name);
			send(fd1, buf_send, strlen(buf_send), 0);

			printf("start\n");
			game_time = 0;

			char buf_send2[300];
			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[0], game_board[1], game_board[2]);
			send(fd1, buf_send2, strlen(buf_send2), 0);
			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[3], game_board[4], game_board[5]);
			send(fd1, buf_send2, strlen(buf_send2), 0);
			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[6], game_board[7], game_board[8]);
			send(fd1, buf_send2, strlen(buf_send2), 0);
			send(fd1,"Please enter number(#0~8)\n",strlen("Please enter number(#0~8)\n"),0);
			//printf("%s", buf_send2);
		}
		else if(buf2[0] == '#'){
			int n=atoi(&buf2[1]);
			int fd2 = game_list[fd_user][1];
			game_time++;
			if(game_board[n] == ' '){
				game_board[n] = 'O';
			}
			

			char buf_send2[300];
			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[0], game_board[1], game_board[2]);
			send(fd2, buf_send2, strlen(buf_send2), 0);
			send(fd_user, buf_send2, strlen(buf_send2), 0);

			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[3], game_board[4], game_board[5]);
			send(fd2, buf_send2, strlen(buf_send2), 0);
			send(fd_user, buf_send2, strlen(buf_send2), 0);

			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[6], game_board[7], game_board[8]);
			send(fd2, buf_send2, strlen(buf_send2), 0);
			send(fd_user, buf_send2, strlen(buf_send2), 0);

			int q = judge();
			if(q == 1){
				send(fd2,"O win!\nEND!\n",strlen("O win!\nEND!\n"),0);
				send(fd_user,"O win!\nEND!\n",strlen("O win!\nEND!\n"),0);
			}
			if(q == 2){
				send(fd2,"X win!\nEND!\n",strlen("X win!\nEND!\n"),0);
				send(fd_user,"X win!\nEND!\n",strlen("X win!\nEND!\n"),0);
			}
			else if(game_time == 9){
				send(fd2,"Fair!\nEND!\n",strlen("Fair!\nEND!\n"),0);
				send(fd_user,"Fair!\nEND!\n",strlen("Fair!\nEND!\n"),0);
			}
			else{
				send(fd2,"Please enter number($0~8)\n",strlen("Please enter number($0~8)\n"),0);
			}
			
		}
		else if(buf2[0] == '$'){
			int n=atoi(&buf2[1]);
			int fd1 = game_list[fd_user][1];
			game_time++;
			if(game_board[n] == ' '){
				game_board[n] = 'X';
			}

			char buf_send2[300];
			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[0], game_board[1], game_board[2]);
			send(fd1, buf_send2, strlen(buf_send2), 0);
			send(fd_user, buf_send2, strlen(buf_send2), 0);

			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[3], game_board[4], game_board[5]);
			send(fd1, buf_send2, strlen(buf_send2), 0);
			send(fd_user, buf_send2, strlen(buf_send2), 0);

			sprintf(buf_send2, "|%c|%c|%c|\n", game_board[6], game_board[7], game_board[8]);
			send(fd1, buf_send2, strlen(buf_send2), 0);
			send(fd_user, buf_send2, strlen(buf_send2), 0);

			int q = judge();
			if(q == 1){
				send(fd1,"O win!\nEND!\n",strlen("O win!\nEND!\n"),0);
				send(fd_user,"O win!\nEND!\n",strlen("O win!\nEND!\n"),0);
			}
			if(q == 2){
				send(fd1,"X win!\nEND!\n",strlen("X win!\nEND!\n"),0);
				send(fd_user,"X win!\nEND!\n",strlen("X win!\nEND!\n"),0);
			}
			else if(game_time == 9){
				send(fd1,"Fair!\nEND!\n",strlen("Fair!\nEND!\n"),0);
				send(fd_user,"Fair!\nEND!\n",strlen("Fair!\nEND!\n"),0);
			}
			else{
				send(fd1,"Please enter number(#0~8)\n",strlen("Please enter number(#0~8)\n"),0);
			}
		}
		else if(strncmp(buf2,"quit",4) == 0){
			for (int i = 0;i < 100;i++){
				if (fd_user_list[i] != 0){
					char buf_send[100] = {};

					if(fd_user_list[i] == fd_user){
						sprintf(buf_send, "USER: %s QUIT SNCCESSFUL\n" ,now_account[fd_user_list[i]].user_name);
						
						fd_user_list[i] = 0;
						send(fd_user, buf_send, strlen(buf_send), 0);
					}
				}
			}
		}


	}
}

int main()
{
    int socket_fd;      /* file description into transport */
    int recfd;     /* file descriptor to accept        */
    int length;     /* length of address structure      */

    char buf[BUFSIZ];

    struct sockaddr_in myaddr; /* address of this service */
    struct sockaddr_in client_addr; /* address of client    */


    /*Get a socket into TCP/IP*/
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) <0) {
        perror ("socket failed");
        exit(1);
    }

    /*Set up address*/
    bzero ((char *)&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(SERV_PORT);

    /*Bind to the address to which the service will be offered*/
    if (bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) <0) {
        perror ("bind failed");
        exit(1);
    }

    /*Set up the socket for listening, with a queue length of 5*/
    if (listen(socket_fd, 20) <0) {
        perror ("listen failed");
        exit(1);
    }

    /*
    * Loop continuously, waiting for connection requests
    * and performing the service
    */
    length = sizeof(client_addr);
    printf("Server is ready to receive !!\n");
    printf("Can strike Cntrl-c to stop Server >>\n");
    printf("myaddress form %s : %d\n", inet_ntoa(myaddr.sin_addr), htons(myaddr.sin_port));

    while (1) {
        if ((recfd = accept(socket_fd,(struct sockaddr_in *)&client_addr, &length)) <0){
            perror ("could not accept call");
            exit(1);
        }

        printf("Create socket #%d form %s : %d\n", recfd, inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
        printf("%s\n", buf);

        for(int i = 0; i < 10; i++){
            if(fd_user_list[i] == 0){
                fd_user_list[i] = recfd;
                pthread_t tid;
				pthread_create(&tid,0,service_thread,&recfd);
				break;
            }
        }
 
        /* return to client */
        /*if (write(recfd, &buf, nbytes) == -1) {
            perror ("write to client error");
            exit(1);
        }*/
        //close(recfd);
        printf("Can Strike Crtl-c to stop Server >>\n");
    }

    return 0;
}

