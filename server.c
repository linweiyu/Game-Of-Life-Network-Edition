#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include "Cell.h"
// Server log identifier
int logid;
// all used character
char CanUse[CharacterNumber]={'O','X','*','Z'};
// now character index can use
int now=0;
//all clients socket list 
struct clientsocket clients[ClientNumber];
int nowclientindex=0;
//real matrix
int realmatrix[35][90];
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
//say ok to clients
void SayOK(int csock);
//deal content from client
void deal(int (*matrix)[90]);

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
	void sendtoclient(int csock);
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
				SayOK(csock);
				close(csock);
				leaveoneclient(csock);
				return;
			case 2:
				deal(newmessage.content);
				sendtoclient(csock);
				break;
			case 9:
				sendtoclient(csock);
				break;
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
void SayOK(int csock){
	struct message sendclient;
	sendclient.type=8;
	strcpy(sendclient.description,"OK!");
	write(csock,&sendclient,sizeof(sendclient));
}
void deal(int (*matrix)[90]){
	int i,j;
	for(i=0;i<35;i++){
		for(j=0;j<90;j++){
			if(matrix[i][j]==1){
				realmatrix[i][j]=1;
			}
		}
	}

}
void sendtoclient(int csock){
	void tick();
	struct message transres;
	transres.type=3;
	tick();
	int i,j;
	for(i=0;i<35;i++){
		for(j=0;j<90;j++){
			transres.content[i][j]=realmatrix[i][j];
		}
	}
	write(csock,&transres,sizeof(transres));

}

void tick()
{
	int neighbours(int y, int x);
    /* t = current tick iteration
     * x = x coordinate
     * y = y coordinate
     * n = neighbours for a coordinate
     */
    int t, x, y, n;
    /* Sync the buffer with the game area
     * buffer <- cells.
     * The buffer and cells are separated so that the calculations from this
     * round won't affect this round. For example let's imagine that coordinate
     * (y,x) (1,1) is active but is deemed dead after visiting it. Then we
     * check the coordinate (y,x) (2,1) which now has only 1 neighbour and also
     * dies. By separating the cells and buffer we keep the cells untouchable
     * during the round, but after we have finished the round, we can sync
     * them. */
	int ticksize=1;
    for(t = 0; t < ticksize; t++)
    { /* Iterations */
        for(x = 0; x < 90; x++)
        {
            for(y = 0; y < 35; y++)
            {
                /* Get the neighbours for the coordinate */
                n = neighbours(y, x);
                /* If a cell has less than two neighbours it dies, no matter
                 * what. If a cell has more than three neighbours it's
                 * overcrowded and dies too. */
                if(n < 2 || n > 3)
                {
					realmatrix[y][x]=0;
                }
                /* If a cell has 3 neighbours it revives */
                if(n == 3)
                {
                    realmatrix[y][x]=1;

                }
                /* The only possibility left is that the cell has two
                 * neighbours, which means that the cell stays alive if it's
                 * alive, if it's dead it stays dead. */
            }
        }
    }
}

int neighbours(int y, int x)
{
	int ALIVE(int y,int x);
    int n = 0; /* Alive neighbours */

    /* The curses coordinate system puts y=0 to the top, so y-1 means checking
     * for above and y+1 means the bottom */

    /* Check for boundaries and then sum up the alive cells */
    if(y < 35)
    {
        n += ALIVE(y+1,x); /* Bottom */
    }
    if(y > 0)
    {
        n += ALIVE(y-1,x); /* Top */
    }
    if(x < 90)
    {
        n += ALIVE(y, x+1); /* Right */
        if(y > 0)
        {
            n += ALIVE(y-1, x+1); /* Top right */
        }
        if(y < 35)
        {
            n += ALIVE(y+1, x+1); /* Bottom right */
        }
    }
    if(x > 0)
    {
        n += ALIVE(y, x-1); /* Left */
        if(y > 0)
        {
            n += ALIVE(y-1, x-1); /* Top left */
        }
        if(y < 35)
        {
            n += ALIVE(y+1, x-1); /* Bottom left */
        }
    }

    return n;
}
int ALIVE(int y,int x){
	return realmatrix[y][x]==1?1:0;
}
