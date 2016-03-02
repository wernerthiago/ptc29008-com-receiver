//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <termios.h>
//#include <iostream>
//#include <sys/types.h>
//
//using namespace std;
//
//bool send(int tty_fd, char data, int count, int length){
//	char flag = 0x7E;
//	char esc = 0x7D;
//	switch(count){
//	case 1:
//		write(tty_fd,&flag,1);
//		write(tty_fd,&data,1);
//		cout << "Case 1: " << flag << endl;
//		cout << "Case 1: " << data << endl;
//		break;
//	default:
//		if((data == 0x7E) && (count != length)){
//			write(tty_fd,&esc,1);
//			cout << esc << endl;
//			write(tty_fd,&data,1);
//			cout << "Primeira condição: " << data << endl;
//		}else{
//			if(count == length){
//				write(tty_fd,&data,1);
//				cout << "Segunda condição: " << data << endl;
//				write(tty_fd,&flag,1);
//				cout << "Segunda condição: " << flag << endl;
//			}else{
//				write(tty_fd,&data,1);
//				cout << "Terceira condição: " << data << endl;
//			}
//		}
//		break;
//	}
//}
//
//int main(){
//	struct termios tio;
//	struct termios stdio;
//	struct termios old_stdio;
//	int tty_fd;
//
//	unsigned char c='D';
//	tcgetattr(STDOUT_FILENO,&old_stdio);
//
//	cout << "Enviando dados!" << endl;
//	memset(&stdio,0,sizeof(stdio));
//	stdio.c_iflag=0;
//	stdio.c_oflag=0;
//	stdio.c_cflag=0;
//	stdio.c_lflag=0;
//	stdio.c_cc[VMIN]=1;
//	stdio.c_cc[VTIME]=0;
//	tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
//	tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
//	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking
//
//	memset(&tio,0,sizeof(tio));
//	tio.c_iflag=0;
//	tio.c_oflag=0;
//	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
//	tio.c_lflag=0;
//	tio.c_cc[VMIN]=1;
//	tio.c_cc[VTIME]=5;
//
//	tty_fd=open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);
//	cfsetospeed(&tio,B9600);            // 115200 baud
//	cfsetispeed(&tio,B9600);            // 115200 baud
//
//	char * msg = "Falador";
//	int i = 1;
//	while(i < strlen(msg)+1){
//		send(tty_fd,msg[i-1],i,strlen(msg));
//		i++;
//	}
//	sleep(4);
//	close(tty_fd);
//
//	return EXIT_SUCCESS;
//}

#include "APC220.h"

using namespace std;

int main(){
	APC220 receiver;

	char * data = "0";
	data = receiver.receiveFSM();
	cout << data << endl;
	receiver.closed();
	return 0;
}
