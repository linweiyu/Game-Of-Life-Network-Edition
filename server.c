#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "Cell.h"
// Server log identifier
int logid;

//error control
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
//deal message from client
void processms(int csock,struct message transfermes);
//write some running info to log.txt
void writetolog(char* content);

int main(int argc, char *argv[])
{
	struct message transfermes;
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	//open log. Record the log.
	logid=open("log.txt",O_WRONLY);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);

	writetolog("Server begin to listen a port");
	
	while(1){
		newsockfd = accept(sockfd,NULL,NULL);
		if (newsockfd < 0) 
			error("ERROR on accept");
		bzero(&transfermes,sizeof(transfermes));
		n = read(newsockfd,&transfermes,sizeof(transfermes));
		if (n < 0) error("ERROR reading from socket");
		processms(newsockfd,transfermes);
	}

	close(sockfd);
	close(logid);
	return 0; 
}

void processms(int csock,struct message transfermes){
	void getplayers(int csock,struct message transfermes);
	void OK(int csock);
	
	struct message newmessage;
	newmessage=transfermes;
	int n;
	if(fork()!=0)
		return;
	while(1){
		switch(newmessage.type){
			case 0:
				writetolog("Get A request want to playerslist \n");
				getplayers(csock,transfermes);
				break;
			case 9:
				writetolog("One Client Leaved \n");
				OK(csock);
				close(csock);
				return;
			default:
				break;
		}
		bzero(&newmessage,sizeof(newmessage));
		n=read(csock,&newmessage,sizeof(newmessage));
		if(n<0) error("ERROR reading from socket");
	}
	
}

//first time communicate
void getplayers(int csock,struct message transfermes){
	struct message toclient;
	char buffer[256];
	toclient.type=1;
	int playersfd,bufn;
	
	playersfd=open("players",O_RDONLY);
	while((bufn=read(playersfd,&buffer,256))>0){
		strncat(toclient.content,buffer,bufn);
	}
	write(csock,&toclient,sizeof(toclient));
	close(playersfd);
}
void OK(int csock){
	struct message toclient;
	toclient.type=2;
	strcpy(toclient.content,"OK");
	write(csock,&toclient,sizeof(toclient));
}
void writetolog(char* content){
	write(logid,content,strlen(content));
}
