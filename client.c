#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <curses.h>
 
#define BUFSIZE 4096
struct User{
	int fd;
	int roomId;
	int userSide;
};
struct Room{
	struct User user0;
	struct User user1;
	int full;	
	int f[16][63];
	int boll[2];
	int leftPlayerX;
	int rightPlayerX;
	int scoreA;
	int scoreB;
	int length;
	int sizing;
	int dirX;
	int dirY;
	int speed;
	int lMark;
	int rMark;
	int changed;
};
struct Message{
	char cmove;
};
char buffer_send[BUFSIZE] = {0};
char buffer_recv[BUFSIZE] = {0};
int finish=0;
//int getch(); 
// send to server
void * sendMsg(void *socket){
	char cmove = 10;
	int * client_socket = (int *)socket;
	struct Message message;
	while(1){
		memset(&message,0,sizeof(message));
		cmove = getch();
		message.cmove=cmove;
		usleep(20000);		
		if(send(*client_socket, &message, sizeof(message), 0)>0){	
		}
		
		// is exit
		if(finish){
			break;
		}
	}
	pthread_exit(NULL);
}
 
void * recvMsg(void *socket){
	int * client_socket = (int *)socket;
	char cmove;
	while(1){
		bzero(buffer_recv, BUFSIZE);
		struct Room room;
		memset(&room,0,sizeof(room));
		// read data from client
		if(recv(*client_socket, &room, sizeof(room), 0) > 0){
			for (int i = 0; i < 16; i++){
				move(3 + i, 1);
				for (int j = 0; j < 63; j++){
					if (!room.f[i][j]){	
						printw(" ");
					}else if(room.f[i][j]==1){	
						attron(COLOR_PAIR(3));
						printw("o");
					}else if(room.f[i][j]==2){
						attron(COLOR_PAIR(2));
						printw("o");
					}
				}
			}
			attron(COLOR_PAIR(4));
			move(20, 12);
			printw("%d", room.scoreA);
			move(20, 58);
			printw("%d", room.scoreB);
			mvaddch(3 + room.boll[1], 1 + room.boll[0], '*');
			refresh();
			if(room.scoreA==9 || room.scoreB==9){
				finish=1;
				break;
			}
		}
	}
	pthread_exit(NULL);
} 
int main(int ac, char *av[]){
	// create socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	// requset server
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));// fill zero
	server_addr.sin_family = AF_INET;// use IPV4 address
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");// ip
	server_addr.sin_port = htons(atoi(av[1]));// port
	connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	WINDOW* win = initscr();
	halfdelay(1.5);
	curs_set(0);
	noecho();

	
	if (has_colors())
	{
		start_color();
		init_pair(1, COLOR_CYAN, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	}


	attron(COLOR_PAIR(1));
	move(0, 0);
	printw("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	printw("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	printw("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("x                                                               x\n");
	printw("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	printw("  A Score :        xxxxxxxxxxxxxxxxxxxxxxxxxx  B Score :        \n");
	printw("  xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	

	// clear buffer_recv
	bzero(buffer_recv, BUFSIZE);			
	
	// create send and recv thread
	pthread_t send_thread, recv_thread;
	
	pthread_create(&send_thread, NULL, sendMsg, (void *)&client_socket);
	pthread_create(&recv_thread, NULL, recvMsg, (void *)&client_socket);
 
	// wait send and recv thread over
	pthread_join(send_thread, NULL);
	pthread_join(recv_thread, NULL);
	
	/*
	clear();
	refresh();
	endwin();
	*/	
	// close
	clear();
	refresh();
	endwin();
	
	close(client_socket);
	
	return 0;
}
