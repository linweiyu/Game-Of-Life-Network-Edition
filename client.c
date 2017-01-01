#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <curses.h>
#include <signal.h>
#include "Cell.h"
#define IPADDR "127.0.0.1"
#define PORT 10086


//  socket between server and socket
int SocketClient;
//	global variable
struct message global_ms;

void error(const char *msg){
	perror(msg);
	exit(0);
}
// connect to server
int connectserver(){
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(IPADDR);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(PORT);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		error("ERROR connecting");
		exit(0);
	}
	return sockfd;
}

// first time init
void firstinit(){
	struct message transfermes;
	int n;

	SocketClient=connectserver();
	transfermes.type=0;
	strcpy(transfermes.description,"Hello");
	n = write(SocketClient,&transfermes,sizeof(transfermes));
	if (n < 0)
		error("ERROR writing to socket");
	bzero(&transfermes,sizeof(transfermes));
	n = read(SocketClient,&transfermes,sizeof(transfermes));
	if (n < 0)
		error("ERROR reading from socket");
	printf("%s\n",transfermes.description);
	//close(sockfd);
	//resolve
	
	//draw 
	initscr();
	noecho();
}
// game end
void finsh(){
	struct message end;
	int n;
	end.type=9;
	strcpy(end.content,"Good Bye!");
	write(SocketClient,&end,sizeof(end));
	if(n<0){
		error("ERROR writing to socket");
	}
	bzero(&end,sizeof(end));
	n=read(SocketClient,&end,sizeof(end));
	if(end.type!=2){
		error("Something wrong in server");
	}
	close(SocketClient);
	move(50,50);
	addstr("Good Bye");
	refresh();
	sleep(2);
	// echo();
	endwin();
}


int main(int argc, char *argv[]){
	firstinit();
	char command;
	while(1){

		command=getch();
		switch(command){
			case 'q':
				finsh();
				break;

		}

	}
	
	return 0;
}
