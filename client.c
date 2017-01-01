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
//info width
# define ICOLS 30
# define ILINES LINES

//  socket between server and socket
int SocketClient;
//	global variable
struct message global_ms;
//	character express
char express;
//activate coordinate
void activate(int y, int x);
//send coordinate to server
void sendclientinfo();
//curses 
/* Size of the 'game area' */
static int lifecols = 0;
static int lifelines = 0;
/* Information window */
static WINDOW *info = NULL;
/* Game area */
static WINDOW *life = NULL;
/* create window */
WINDOW *createwin(int height, int width, int begy, int begx);



# define CPR(y, x) (global_ms.content[y][x] = 1) /* Give CPR to the defined
                                                  coordinate. (Make it alive)
                                                  */
# define LMAX (lifelines-1) /* Maximum lines */
# define CMAX (lifecols-1) /* Maximum columns */

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
	system("resize -s 35 121");
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
	//printf("%s\n",transfermes.description);
	
	if(transfermes.type==9){
		printf("Server Meet Some Problems. Please try again later ! \n");
		return;
	}
	else{
		bzero(&global_ms,sizeof(global_ms));
		printf("Get Server Character\n");
		express=transfermes.description[0];
		printf("%c \n",express);
		
		/* Initialize curses */
		initscr();
		/* Don't echo keys */
		noecho();
		raw();
		/* Allow arrows and other weird keys */
		keypad(stdscr, true);
		/* Refresh the standard screen */
		refresh();

		lifecols = COLS-ICOLS-1; /* Leave one column empty */
		lifelines = LINES; /* Full height */

		/* Create the two windows */
		life = createwin(lifelines, lifecols, 0, 0);
		info = createwin(ILINES, ICOLS, 0, lifecols+1);

		/* Make a default box around the info-window */
		box(info, 0, 0);
		/* Move, window, printw; Write to window info and move the cursor to 1,1
		* before writing. */
		mvwprintw(info, 1,1, "INFO");

		/* Move the cursor back to game area. The user don't need to see the cursor
		* jumping to the info-window */
		wmove(life, 1,1);

		/* Refresh both of the windows */
		wrefresh(info);
		wrefresh(life);

	}
}
// game end
void finsh(){
	struct message end;
	int n;
	end.type=6;
	strcpy(end.description,"Good Bye!");
	write(SocketClient,&end,sizeof(end));
	if(n<0){
		error("ERROR writing to socket");
	}
	bzero(&end,sizeof(end));
	n=read(SocketClient,&end,sizeof(end));
	if(end.type!=8){
		error("Something wrong in server");
	}
	close(SocketClient);
	refresh();
	sleep(2);
	/* Delete the two windows we made */
    delwin(info);
    delwin(life);
    /* And end curses mode. It's bad if this is forgotten. Vey very bad */
    endwin();
	exit(0);
}


int main(int argc, char *argv[]){
	void watch();
	firstinit();
	int x,y; /* Holds the coordinates */
    getyx(life, y, x); /* Get the coordinates from curses life window */
	char command;
	while(1){

		command=getch();
		switch(command){
			case 'q':
				finsh();
				break;
			case 'l': /* Go right */
				x=(x+1)%CMAX;
				break;
			case 'h': /* Go left */
				x=(x-1)%CMAX;
				break;
			case 'j': /* Go down */
				y=(y+1)%LMAX;
				break;
			case 'k': /* Go up */
				y=(y-1)%LMAX;
				break;
			case ' ': /* Activate a cell */
				activate(y, x);
				break;
			case 10:
				sendclientinfo();
				watch();
				break;
		}
		wmove(life, y, x); /* Move the coordinates to the new location and refresh
                          the game area */
    	wrefresh(life);

	}
	
	return 0;
}

void watch(){
	void refreshnewresult(int (* matrix)[90]);
	int n;
	char command;
	struct message transfer;
	while(1){
		
		n=read(SocketClient,&transfer,sizeof(transfer));
		if(n>0&&transfer.type==3)
			refreshnewresult(transfer.content);
		bzero(&transfer,sizeof(transfer));
		transfer.type=9;
		strcpy(transfer.description,"Continue");
		write(SocketClient,&transfer,sizeof(transfer));

	}

}
void refreshnewresult(int (* matrix)[90]){
	int i,j;
	for(i=0;i<35;i++){
		for(j=0;j<90;j++){
			if(matrix[i][j]==1){
				mvwaddch(life, j, i, express);
			}else{
				mvwaddch(life, j, i, " ");
			}
		}
	}

}



WINDOW *createwin(int height, int width, int begy, int begx)
{
    /* Hold the window */
    WINDOW *local_win = NULL;

    /* Create the window */
    local_win = newwin(height, width, begy, begx);
    if(local_win == NULL) /* Check that it succeeded */
        exit(EXIT_FAILURE);

    wmove(local_win, 1, 1); /* Looks better if the cursor is not on
                               top of lines */

    wrefresh(local_win); /* And refresh */

    return local_win;
}
void activate(int y, int x)
{
    /* Set the cell alive in the array */
    CPR(y,x);
    /* And show it visually too */
    mvwaddch(life, y, x, express);
}
void sendclientinfo(){
	global_ms.type=2;
	strcpy(global_ms.description,"All Coordinate Send to Server");
	write(SocketClient,&global_ms,sizeof(global_ms));
	bzero(&global_ms,sizeof(global_ms));
}
