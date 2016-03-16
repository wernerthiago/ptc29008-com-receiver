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
	seq = 0;

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

	this->tty_fd=open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK);
	cfsetospeed(&tio,B9600);            // 115200 baud
	cfsetispeed(&tio,B9600);            // 115200 baud

	tcsetattr(this->tty_fd,TCSANOW,&tio);
}

APC220::~APC220() {
	// TODO Auto-generated destructor stub
}

void APC220::closed() {
	close(tty_fd);
}

char * APC220::receiveFSM() {
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
				return buffer;
			}else{
				cout << "Receive = 0, mas não terminou a mensagem!" << endl;
				cout << "Erro inesperado!" << endl;
				return buffer;
			}
		default:
			switch(data){
			case 0x7E:
				if(bandeira == true){
					bandeira = false;
				}else{
					teste = buffer;
					//buffer[i++] = '\0';
					return buffer;
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
				cout << data << endl;
				i++;
				break;
			}
			break;
		}
	}
}

bool APC220::send(char* msg) {
	int i = 1;
	string mensagem;

	//Adicionando a SEQ
	//	mensagem = msg;
	//	mensagem.insert(0,1,getPTC());
	msg[2] = getPTC();
	//Adicionando o PROTOCOLO
	if(getSEQ() == 1){
		//		mensagem.insert(0,1,'1');
		msg[1] = '1';
	}else{
		//		mensagem.insert(0,1,'0');
		msg[1] = '0';
	}
	//Adicionando o TYPE da mensagem
	//Se 0 é dados
	//Se 1 é controle
	//	mensagem.insert(0,1,'0');
	msg[0] = '0';
	//	char * aux = new char[mensagem.length() + 1]; // or
	//	std::strcpy(aux, mensagem.c_str());
	unsigned short fcs = (uint16_t)this->crcFast(msg,strlen(msg));
	char lo = fcs & 0xFF;
	char hi = fcs >> 8;
	//Adicionando CRC
	char * payload = appendCharToCharArray(msg,lo);
	payload = appendCharToCharArray(payload,hi);
	cout << payload << endl;
	//Enviando
	while(i < (strlen(payload)+1)){
		sendFSM(tty_fd,payload[i-1],i,strlen(payload));
		//		cout << "payload[i-1]: " << payload[i-1] << endl;
		sleep(1);
		i++;
	}

	if(timeout()){
		char * arq = receiveFSM();
		//		cout << "ARQ: " << arq << endl;
		arq = crcReception(arq);
		arq = headerReception(arq);
		if(arq == "1"){
			send(msg);
		}
	}else{
		cout << "Sem resposta do receptor" << endl;
		send(msg);
	}
	cout << "ACK" << getSEQ() << endl;
	setSEQ(!getSEQ());
	return true;
}

bool APC220::sendControl() {
	int i = 1;
	char * mensagem;

	//Adicionando a SEQ
	if(getSEQ() == 1){
		mensagem[3] = '1';
	}else{
		mensagem[3] = '0';
	}
	//Adicionando o TYPE da mensagem
	//Se 0 é dados
	//Se 1 é controle
//	mensagem.insert(0,1,'0'); //ENCHIMENTO
	mensagem[2] = '0';
//	mensagem.insert(0,1,'0'); //ENCHIMENTO
	mensagem[1] = '0';
//	mensagem.insert(0,1,'1');
	mensagem[0] = '1';
	//	cout << "ACK Frame: " << mensagem << endl;
//	char * aux = new char[mensagem.length() + 1]; // or
//	std::strcpy(aux, mensagem.c_str());
	//	cout << "strlen(aux): " << strlen(aux) << endl;
	unsigned short fcs = (uint16_t)this->crcFast(mensagem,strlen(mensagem));
	char lo = fcs & 0xFF;
	//	cout << "ACK Low bit: " << unsigned(lo) << endl;
	char hi = fcs >> 8;
	//	cout << "ACK High bit: " << unsigned(hi) << endl;
	//Adicionando CRC
	char * payload = appendCharToCharArray(mensagem,lo);
	payload = appendCharToCharArray(payload,hi);
	//	cout << "ACK+FCS: " << payload << endl;
	//Enviando
	while(i < (strlen(payload)+1)){
		sendFSM(tty_fd,payload[i-1],i,strlen(payload));
		//		cout << "payload[i-1]: " << payload[i-1] << endl;
		sleep(1);
		i++;
	}
	return true;
}

char * APC220::appendCharToCharArray(char* array, char a){
	size_t len = strlen(array);

	char* ret = new char[len+2];

	strcpy(ret, array);
	ret[len] = a;
	ret[len+1] = '\0';

	return ret;
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

char * APC220::crcReception(char * teste){
	char rec_lo = teste[strlen(teste)-2];
	char rec_hi = teste[strlen(teste)-1];
	string mensagem = teste;
	mensagem.resize(mensagem.size()-2);
	char *cstr = new char[mensagem.length() + 1];
	strcpy(cstr, mensagem.c_str());
	unsigned short fcs = (uint16_t)crcFast(cstr,strlen(cstr));
	char lo = fcs & 0xFF;
	char hi = fcs >> 8;
	if(lo == rec_lo){
		if(hi == rec_hi){
			cout << "CRC: OK" << endl;
		}else{
			cout << "CRC: NOK" << endl;
			cout << "CRC: High bit error" << endl;
		}
	}else{
		cout << "CRC: NOK" << endl;
		cout << "CRC: Low bit error" << endl;
	}
	return cstr;
}

bool APC220::timeout(){
	struct timeval timeout; // para especificar o timeout
	timeout.tv_sec = 4; //timeout de 2 segundos
	timeout.tv_usec = 0;

	fd_set espera;
	FD_ZERO(&espera);
	FD_SET(tty_fd, &espera);

	if (select(tty_fd+1, &espera, NULL, NULL, &timeout) == 0) { // timeout !!
		cout << "TIMEOUT" << endl;
		return false;
	} else {
		cout << "RECEBENDO" << endl;
		return true;
	}
}

char * APC220::headerReception(char * mensagem){
	int aux;
	char * aux2;

	if(mensagem[0] == '0'){
		//		cout << "Mensagem de dados" << endl;
		if(mensagem[1] == '0'){
			aux = 0;
		}else{
			aux = 1;
		}
		if(aux != getSEQ()){
			setSEQ(mensagem[1]);
			setPTC((Protocol)mensagem[2]);
			aux2 = mensagem+3;
		}else{
			cout << "Descarta a mensagem" << endl;
			return "0";
		}
	}else{
		if(mensagem[3] == '0'){
			aux = 0;
		}else{
			aux = 1;
		}
		if(aux == getSEQ()){
			setSEQ(aux);
			aux2 = mensagem+4;
		}else{
			cout << "Retransmite" << endl;
			return "1";
		}
	}
	return aux2;
}
