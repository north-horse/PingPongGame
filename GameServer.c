#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
# define MAXROOM 10

pthread_mutex_t mut;
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
}Room;
struct Message{
	char cmove;
};
void changeDirection(struct Room *room){
	if(room->changed==0){
		pthread_mutex_lock(&mut);
		switch (room->dirX){
			case 1:
				room->boll[0]++;
				break;
			case 0:
				break;
			case -1:
				room->boll[0]--;
				break;
		}
		switch (room->dirY){
			case 1:
				room->boll[1]++;
				break;
			case 0:
				break;
			case -1:
				room->boll[1]--;
				break;
		}
		room->changed=1;
		//room->f[1][1] = 4;
		pthread_mutex_unlock(&mut);
	}else{
		pthread_mutex_lock(&mut);
		room->changed=0;
		//room->f[1][1] = 5;
		pthread_mutex_unlock(&mut);
	}
}
void Run(struct Room *room){
		pthread_mutex_lock(&mut);
		memset(room->f, 0, 16*63*sizeof(int));
		room->f[1+room->leftPlayerX][0] = 1;
		room->f[2+room->leftPlayerX][0] = 1;
		room->f[3+room->leftPlayerX][0] = 1;
		room->f[4+room->leftPlayerX][0] = 1;
		room->f[5+room->leftPlayerX][0] = 1;
		room->f[1+room->rightPlayerX][62] = 2;
		room->f[2+room->rightPlayerX][62] = 2;
		room->f[3+room->rightPlayerX][62] = 2;
		room->f[4+room->rightPlayerX][62] = 2;
		room->f[5+room->rightPlayerX][62] = 2;
		pthread_mutex_unlock(&mut);

}
void hasWon(struct Room *room){
	int won = 0;
	if(room->boll[0] == 0 && room->f[room->boll[1]][room->boll[0]]){
		room->dirX = 1;
		if(room->lMark == 1){
			room->dirY = 1;			
		}else if(room->lMark == -1){
			room->dirY = -1;
		}			
	}
	if(room->boll[0] == 62 && room->f[room->boll[1]][room->boll[0]]){
		room->dirX = -1;
		if(room->rMark == 1){
			room->dirY = 1;			
		}else if(room->rMark == -1){
			room->dirY = -1;
		}
	}
	if (room->boll[0] < 0){
		room->scoreB ++;
		room->boll[1] = 7;
		room->boll[0] = 32;
		room->leftPlayerX = 0;
		room->rightPlayerX = 0;
		room->dirX = 1;
		room->dirY = 0;
	}
	if(room->boll[0] > 62){
		room->scoreA ++;
		room->boll[1] = 7;
		room->boll[0] = 32;
		room->leftPlayerX = 0;
		room->rightPlayerX = 0;
		room->dirX = 1;
		room->dirY = 0;
	}
	if(room->boll[1] == 0){
		room->dirY = 1;
	}
	if(room->boll[1] == 15){
		room->dirY = -1;
	}
}
void InitStruct(struct Room *room){
	pthread_mutex_lock(&mut);
	memset(room->f, 0, 16*63*sizeof(int));
	room->boll[1] = 7;
	room->boll[0] = 32;
	room->leftPlayerX = 0;
	room->rightPlayerX = 0;
	room->scoreA = 0;
	room->scoreB = 0;
	room->length = 5;
	room->sizing = 0;
	room->dirX = 1;
	room->dirY = 0;
	room->speed = 2;
	room->lMark = 0;
	room->rMark = 0;
	room->changed = 0;
	//room->f[1][1] = 3;
	pthread_mutex_unlock(&mut);
}
void *handle_recv(void *args){
	char buf[1024];
	//int start = 0;
	int n;
	int fd;
	int roomId;
	int userSide;
	char cmove;
	struct User *user;
	struct Message message;
	user = (struct User *)args;
	fd=user->fd;
	roomId=user->roomId;
	userSide = user->userSide;
	///printf("KKKKKKKK");
	while(1){
		memset(&message,0,sizeof(message));
		if((recv(fd, &message, 4096, 0))>0){
			if(message.cmove>0){
				cmove = message.cmove;
				pthread_mutex_lock(&mut);
				if(userSide==0){
					printf("player1_1 %c\n", cmove);
					if(cmove=='s' && Room.leftPlayerX <= 10){
						Room.leftPlayerX++;
						Room.lMark = -1;
						printf("%d %c player1\n", fd, cmove);
					}else if(cmove=='w'&& Room.leftPlayerX>=0){
						Room.leftPlayerX--;
						Room.lMark = 1;
						printf("%d %c player1\n", fd, cmove);
					}
					printf("player1 %c\n", cmove);
				}else{
					if(cmove=='i' && Room.rightPlayerX >=0){
						Room.rightPlayerX--;
						Room.rMark = 1;
						printf("%d %c player2\n", fd, cmove);
					}else if(cmove=='k' && Room.rightPlayerX <=10){
						Room.rightPlayerX++;
						Room.rMark = -1;
						printf("%d %c player2\n", fd, cmove);
					}
					printf("player2\n");
				}

				pthread_mutex_unlock(&mut);
				printf("%d %d %d\n", fd, message.cmove, userSide);
			}
		}
		if(Room.scoreA==9 || Room.scoreB==9){
			break;
		}
		//printf("%d %d\n", message.cmove, fd);	

	}

	close(fd);
	pthread_exit(NULL);
}
void *handle_send(void *args){
	
	char buf[4096];
	int start = 0;
	int n;
	int fd;
	int roomId;
	int userSide;
	struct User *user;
	user = (struct User *)args;
	fd=user->fd;
	roomId = user->roomId;
	userSide = user->userSide;


	//printf("%d %d %d \n", fd, user->roomId, user->userSide);
	while(1){		
		if(start==1){
			struct Message message;
			memset(&message,0,sizeof(message));
			usleep(100000);
			Run(&Room);
			changeDirection(&Room);
			hasWon(&Room);
			if((send(fd, &Room, sizeof(buf), 0))>0){
				//printf("s %d %d %d\n", Room[roomId].boll[0], Room[roomId].boll[1], fd);	
			}	
		}
		if(Room.full==1 && start==0){				
			InitStruct(&Room);

			send(fd, &Room, sizeof(buf), 0);
			start++;
		}	
		if(Room.scoreA==9 || Room.scoreB==9){
			send(fd, &Room, sizeof(buf), 0);
			break;
		}

		

	}

	close(fd);
	pthread_exit(NULL);
}

int main(int ac, char *av[]){
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);
	int roomNumber=0;
	int userSide=0;
	int tcp_socket;
	struct sockaddr_in sockaddr;
	int fd;
	pthread_t t;
	tcp_socket  =  socket(AF_INET,  SOCK_STREAM, 0);

	sockaddr.sin_family=AF_INET;
	sockaddr.sin_port=htons(atoi(av[1]));
	sockaddr.sin_addr.s_addr=INADDR_ANY;

	if(bind(tcp_socket, (const struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in))==-1){
		perror("cannot bind");
		exit(1);
	}

	listen(tcp_socket, 1);
	while(1){
		fd=accept(tcp_socket, NULL, NULL);
		//pthread_mutex_lock(&mut);	
		struct User user;
		user.roomId = roomNumber; 
		user.userSide = userSide;
		user.fd = fd;
		printf("rID %d uS %d fd %d\n", user.roomId, user.userSide, user.fd);
		if(userSide==0){
			Room.user0=user;
			Room.full=0;
		}else{
			Room.user1=user;
			Room.full++;
		}
		
		if(userSide == 1){
			roomNumber++;
			userSide--;
		}else{
			userSide++;
		}
		//pthread_mutex_unlock(&mut);
		pthread_create(&t, NULL, handle_send, &user);
		pthread_create(&t, NULL, handle_recv, &user);

	}
}
