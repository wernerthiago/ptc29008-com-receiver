#include "APC220.h"
#include "tun.h"
#include <vector>

using namespace std;

int main(){
	APC220 receiver;
	char * data = 0;
	string teste;
	string mensagem;
	vector<string> buffer;
	int i=0;
	tun tun;

	receiver.setSEQ(1);

	while(1){
		teste = (string)receiver.receiveFSM();
		cout << "Frame: " << teste << endl;
		mensagem = receiver.crcReception(teste);
		cout << "Frame: " << mensagem << endl;
		mensagem = receiver.headerReception(mensagem);
		if(mensagem == "0"){
			cout << "Payload descartado." << endl;
		}else{
			cout << "Payload: " << mensagem << endl;
			buffer.push_back(mensagem);
			i++;
			if(i==2){
				tun.tun_alloc("tun0");
				if (tun.set_ip("tun0", "10.0.0.1", "10.0.0.2") < 0) {
					perror("so configurar a interface tun");
					return 0;
				}
			}
		}

		if(receiver.sendControl()){
			cout << "ACK: OK" << endl;
		}
	}

	receiver.closed();
	return 0;
}
