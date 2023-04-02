#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <iostream>
#include <thread>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "mainwindow.h"
#include <QThread>
#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <string>
#include <string.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/dh.h>
#include <cryptopp/dh2.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/nbtheory.h>
#include <cryptopp/aes.h>
#include <cryptopp/rijndael.h>
#include <sstream>
#include <cryptopp/rsa.h>
#include <cryptopp/modes.h>
#define MAX 2048
using namespace CryptoPP;

class Communication : public QObject
{
    Q_OBJECT
    public:
        int accept_message(int serverSocket, struct sockaddr_in addr, int prejetSocket,char prejetoSporocilo[MAX]);
        void send_message(char[MAX],int,int);
        void create_server(int &, struct sockaddr_in &);
        CryptoPP::SecByteBlock generate_symetric_key(std::string, CryptoPP::SecByteBlock, CryptoPP::SecByteBlock,CryptoPP::DH);
        int sendPort;
        int receivePort;
        char fileName[MAX];
        char ip[MAX];
        Communication(int _sPort, int _rPort, QString _fileName, QString _ip){
            sendPort = _sPort;
            receivePort = _rPort;
            strcpy(fileName, _fileName.toStdString().c_str());
            strcpy(ip, _ip.toStdString().c_str());

        }
        virtual ~Communication();
    public slots:
        void receive_file();
        void send_file();
    signals:
        void finished();
        void update(QString);
        void text_box(QString);
};

#endif // COMMUNICATION_H
