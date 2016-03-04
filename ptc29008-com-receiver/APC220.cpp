/*
 * APC220.cpp
 *
 *  Created on: Feb 23, 2016
 *      Author: werner
 */

#include "APC220.h"

using namespace std;

APC220::APC220() {
	struct termios tio;
	struct termios stdio;

	memset(&stdio,0,sizeof(stdio));
	stdio.c_iflag=0;
	stdio.c_oflag=0;
	stdio.c_cflag=0;
	stdio.c_lflag=0;
	stdio.c_cc[VMIN]=1;
	stdio.c_cc[VTIME]=0;

	memset(&tio,0,sizeof(tio));
	tio.c_iflag=0;
	tio.c_oflag=0;
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
	tio.c_lflag=0;
	tio.c_cc[VMIN]=1;
	tio.c_cc[VTIME]=5;

	this->tty_fd=open("/dev/ttyUSB1", O_RDWR | O_NONBLOCK);
	cfsetospeed(&tio,B9600);            // 115200 baud
	cfsetispeed(&tio,B9600);            // 115200 baud
    tcsetattr(tty_fd,TCSANOW,&tio);
}

APC220::~APC220() {
	// TODO Auto-generated destructor stub
}

void APC220::closed() {
	close(tty_fd);
}

string APC220::receiveFSM() {
	int receive = -1;
	int i = 0;
	bool bandeira = true;
	string teste;
	unsigned char data = '0';
	char buffer[MAX_LENGTH] = {0};

	while(i < MAX_LENGTH){
		receive = read(this->tty_fd,&data,1);
		if(receive != -1){
			sleep(1);
		}else{
			if(receive == 0){
				cout << "receive = 0" << endl;
			}
		}
		switch(receive){
		case -1:
			break;
		case 0:
			if(data == 0x7E){
				cout << "FIM!" << endl;
				return teste;
			}else{
				cout << "Receive = 0, mas não terminou a mensagem!" << endl;
				cout << "Erro inesperado!" << endl;
				return teste;
			}
		default:
			switch(data){
			case 0x7E:
				if(bandeira == true){
					bandeira = false;
				}else{
					cout << buffer << endl;
					teste = buffer;
					//buffer[i++] = '\0';
					return teste;
				}
				break;
			case 0x7D:
				break;
			case 0x5E:
				data = data ^ 0x20;
				buffer[i] = data;
				i++;
				break;
			case 0x5D:
				data = data ^ 0x20;
				buffer[i] = data;
				i++;
				break;
			default:
				buffer[i] = data;
				i++;
				break;
			}
			break;
		}
	}
}


void APC220::send(char* msg) {
	int i = 1;
	while(i < (strlen(msg)+1)){
		sendFSM(tty_fd,msg[i-1],i,strlen(msg));
		sleep(1);
		i++;
	}
	return;
}

void APC220::sendFSM(int tty_fd, char data, int count, int length) {
	char flag = 0x7E;
	char esc = 0x7D;
	switch(count){
	case 1:
		write(tty_fd,&flag,1);//0x7E
		write(tty_fd,&data,1);//Data
		break;
	default:
		if(((data == 0x7E) || (data == 0x7D)) && (count != length)){
			write(tty_fd,&esc,1); //0x7D
			data = data ^ 0x20;
			write(tty_fd,&data,1); //data XOR 0x20
		}else{
			if(count == length){
				write(tty_fd,&data,1);
				write(tty_fd,&flag,1);
			}else{
				write(tty_fd,&data,1);
			}
		}
		break;
	}
}

unsigned short APC220::crcFast(char * message, int nBytes){
	register int counter;
	register unsigned short crc = 0;
	for( counter = 0; counter < nBytes; counter++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(char *)message++)&0x00FF];
	return crc;
}
