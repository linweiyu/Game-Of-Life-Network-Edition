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
// all used character
char CanUse[CharacterNumber]={'O','X','*','@'};
// now character index can use
int now=0; 
//all clients socket list 
struct clientsocket clients[ClientNumber];
int nowclientindex=0;

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
//return the char can use
char getCanUseChar();
//tell client the char can use
void tellCanUseChar(int csock);
//new client add 
void addnewclient(int csock);
//one client leave
void leaveoneclient(int csock);
//find a client 
struct clientsocket* findclient(int csock);

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

	writetolog("Server begin to listen a port \n");
	
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
	
	struct message newmessage;
	newmessage=transfermes;
	int n;
	if(fork()!=0)
		return;
	//add new client to array
	addnewclient(csock);
	while(1){
		writetolog(newmessage.description);
		switch(newmessage.type){
			case 0:
				writetolog("get a request from client \n");
				tellCanUseChar(csock);
				break;
			case 6:
				writetolog("One Client Leaved \n");
				close(csock);
				leaveoneclient(csock);
				return;
			default:
				break;
		}
		bzero(&newmessage,sizeof(newmessage));
		n=read(csock,&newmessage,sizeof(newmessage));
		if(n<0) error("ERROR reading from socket");
	}
	
}

void writetolog(char* content){
	write(logid,content,strlen(content));
}
char getCanUseChar(){
	if(now==CharacterNumber){
		return '~';
	}else{
		return CanUse[now++];
	}
}
void tellCanUseChar(int csock){
	struct message tellclient;
	char canuse;
	canuse=getCanUseChar();

	writetolog(&canuse);
	writetolog("\n");
	struct clientsocket* targetclient;
	targetclient=findclient(csock);
	targetclient->expresschar=canuse;
	if(canuse=='~'){
		tellclient.type=9;
		strcpy(tellclient.description,"Fail");
	}else{
		tellclient.type=1;
		tellclient.description[0]=canuse;

	}
	write(csock,&tellclient,sizeof(tellclient));
}
void addnewclient(int csock){
	int i;
	for(i=0;i<ClientNumber;i++){
		if(clients[i].valid==0){
			clients[i].socketid=csock;
			clients[i].valid=1;
			return;
		}
	}
	error("It reach the maxmium number of client");
}

void leaveoneclient(int csock){
	int i;
	for(i=0;i<ClientNumber;i++){
		if(clients[i].socketid==csock){
			clients[i].valid=0;
			return;
		}
	}
	writetolog("Wrong Client");
}
struct clientsocket* findclient(int csock){
	int i;
	for(i=0;i<ClientNumber;i++){
		if(clients[i].socketid==0){
			return &clients[i];
		}
	}
}