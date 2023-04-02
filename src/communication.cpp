#include "communication.h"

Communication::~Communication(){};


// These are pre-agreed on domain parameters, they could also be generated and send from one user to another.
CryptoPP::Integer p("dc3dda9872bfe172125e0be4914992658100b2bdc122a12ea002ec563093805037b303b501840587b3edf909963ade922d0cab8aea555255170f942f252fe785e08f9274f7f280806137d62fd324e2ab1f3c6e66cf116731500efaf5263f4d2e4e10744b9edde95a73a98c07e4d7599b37842d7104e60c897d08dac48659d07fh");
CryptoPP::Integer q("6e1eed4c395ff0b9092f05f248a4c932c080595ee09150975001762b1849c0281bd981da80c202c3d9f6fc84cb1d6f49168655c5752aa92a8b87ca179297f3c2f047c93a7bf94040309beb17e99271558f9e37336788b398a8077d7a931fa69727083a25cf6ef4ad39d4c603f26baccd9bc216b882730644be846d62432ce83fh");
CryptoPP::Integer g("2.");




CryptoPP::SecByteBlock Communication::generate_symetric_key(std::string receivedMessage, CryptoPP::SecByteBlock privKey, CryptoPP::SecByteBlock shared, CryptoPP::DH dh)
{
    CryptoPP::Integer key;
    CryptoPP::Integer c;
    std::string otherKeyString = receivedMessage;
    std::stringstream ss2;
    ss2 << otherKeyString;
    ss2 >> c;
    CryptoPP::SecByteBlock otherKey;
    size_t encodedSize = c.MinEncodedSize(CryptoPP::Integer::UNSIGNED);
    otherKey.resize(encodedSize);
    c.Encode(otherKey.BytePtr(), encodedSize, CryptoPP::Integer::UNSIGNED);
    if(!dh.Agree(shared, privKey, otherKey)){
        std::cout << "Failed to agree" << std::endl;
        throw -1;
    }
    CryptoPP::SecByteBlock AESkey(shared, CryptoPP::AES::DEFAULT_KEYLENGTH);
    key.Decode(AESkey.BytePtr(), AESkey.SizeInBytes());
    std::cout << "Symmetric key: " << key << std::endl;
    return AESkey;
}



void Communication::send_message(char message[MAX],int portSM, int size){

    if (size == 0){
        size = strlen(message);
    }
    int optval = 1;
    int senderSocket;
    struct sockaddr_in addrC;
    addrC.sin_family = AF_INET;
    addrC.sin_port = portSM;
    if ((senderSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to create sender socket");
        throw -1;
    }
    if(setsockopt(senderSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
        perror("Failed to setsockopt");
        throw -1;
    }
    if(setsockopt(senderSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1){
        perror("Failed to setsockopt");
        throw -1;
    }
    if(inet_pton(AF_INET, this->ip, &addrC.sin_addr) == -1 ){
        perror("Failed to inet_pton");
        throw -1;
    }
    if (::connect(senderSocket, (struct sockaddr *)&addrC, sizeof(addrC)) == -1){
        perror("Failed to connect");
        throw -1;
    }
    if(send(senderSocket, message, size, 0 ) == -1){
        perror("Failed to send");
        throw -1;
    }
    close(senderSocket);
}

void Communication::create_server(int &serverSocket,struct sockaddr_in &addr)
{
    const int *optval = (int*)1;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = this->receivePort;
    if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Failed to create socket");
        throw -1;
    }
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
        perror("Failed to setsockopt");
        throw -1;
    }
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        perror("Failed to bind");
        throw -1;
    }
    printf("Listening on: %s:%d\n", inet_ntoa(addr.sin_addr), this->receivePort);
}


int Communication::accept_message(int serverSocket, struct sockaddr_in addr, int receivedSocket, char receivedMessage[MAX])
{
    int a = 0;
    int addrlen = sizeof(addr);
    if (listen(serverSocket, 0) == -1 ) {
        perror("Failed to listen");
        throw -1;
    }
    if ((receivedSocket = accept(serverSocket, (struct sockaddr *) &addr, (socklen_t *) &addrlen)) == -1) {
        perror("Failed to accept");
        throw -1;
    }
    if ((a = (read(receivedSocket, receivedMessage, MAX))) == -1) {
        perror("Failed to read");
        throw -1;
    }
    close(receivedSocket);
    std::cout << "Received message:" << receivedMessage << std::endl;
    return a;
}


void Communication::receive_file() {
    int serverSocket;
    char receivedMessage[MAX] = {};
    int receivedSocket = 0;
    struct sockaddr_in addr;
    std::string pubKeyStr;
    std::stringstream ss;
    char message[MAX];
    std::string sizeStr;
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::PrimeAndGenerator pg;
    CryptoPP::DH dh(p, q, g);
    CryptoPP::SecByteBlock privKey(dh.PrivateKeyLength());
    CryptoPP::SecByteBlock pubKey(dh.PublicKeyLength());
    CryptoPP::Integer decoder;
    CryptoPP::SecByteBlock shared(dh.AgreedValueLength());

    try {
        emit text_box("Receiving...");

        // GENERATE KEY PAIR
        dh.GenerateKeyPair(prng, privKey, pubKey);


        decoder.Decode(pubKey.BytePtr(), pubKey.SizeInBytes());

        // fill pub key to stream
        ss << decoder;
        ss >> pubKeyStr;


        for(unsigned long i = 0; i < pubKeyStr.length(); i++){
            message[i] = pubKeyStr[i];
        }

        // CREATE SERVER
        create_server(serverSocket, addr);

        // ACCEPT KEY
        accept_message(serverSocket, addr, receivedSocket, receivedMessage);

        // SEND MY KEY
        send_message(message, this->sendPort, 0);

        // GENERATE SYMMETRIC KEY
        CryptoPP::SecByteBlock AESkey = generate_symetric_key(receivedMessage, privKey, shared, dh);

        // GENERATE RANDOM IV
        prng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

        // clear message
        memset(&message[0], 0, sizeof(message));

        // fill IV into message
        for(int i = 0; i < CryptoPP::AES::BLOCKSIZE; i++){
            message[i] = iv[i];
        }

        // SEND message containing IV
        send_message(message, this->sendPort, 0);

        memset(&receivedMessage[0], 0, sizeof(receivedMessage));
        memset(&message[0], 0, sizeof(message));


        // OPEN FILE FOR RECEIVING
        FILE *fp = fopen(this->fileName, "w");
        if(fp == NULL) {
            std::cout << "Failed to open!" << std::endl;
            throw -1;
        }


        // ACCEPT FILE SIZE
        accept_message(serverSocket,addr,receivedSocket,receivedMessage);
        int fullSize = atoi(receivedMessage);
        std::cout << "fullsize: " << fullSize << std::endl;
        int size = 0;
        memset(&receivedMessage[0], 0, sizeof(receivedMessage));

        // INITIALIZE DECRYPTION
        CFB_Mode<AES>::Decryption cfbEncryption(AESkey, AESkey.size(), iv);

        // RECEIVE FILE
        while(fullSize > size) {
            int readSize = accept_message(serverSocket,addr,receivedSocket,receivedMessage);

            // DECRYPT
            cfbEncryption.ProcessData((unsigned char*)receivedMessage,(unsigned char*)receivedMessage,readSize);
            std::cout << "Decrypted message: " << receivedMessage <<"\n Size: " << readSize << std::endl;

            // WRITE FILE
            int writeSize = fwrite(receivedMessage,1,readSize, fp);
            if(readSize != writeSize) {
               std::cout << "Error while writing file" << std::endl;
            }

            size += writeSize;
            std::cout << "Size: " << size << std::endl;
            memset(&receivedMessage[0], 0, sizeof(receivedMessage));
        }

        fclose(fp);
        close(serverSocket);
        emit text_box("File sucessfully received!");
        emit finished();
    }
    catch(int e){
        emit text_box("Something went wrong, try again!");
        std::cout << "Caught an error: " << e << std::endl;
        emit finished();
    }
}


void Communication::send_file() {
    int serverSocket;
    char receivedMessage[MAX] = {};
    int receivedSocket = 0;
    struct sockaddr_in addr;
    std::string pubKeyStr;
    std::stringstream ss;
    char message[MAX];
    std::string sizeStr;
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::PrimeAndGenerator pg;
    CryptoPP::DH dh(p, q, g);
    CryptoPP::SecByteBlock privKey(dh.PrivateKeyLength());
    CryptoPP::SecByteBlock pubKey(dh.PublicKeyLength());
    CryptoPP::Integer decoder;
    CryptoPP::SecByteBlock shared(dh.AgreedValueLength());
    CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];

    try{
        emit text_box("Sending...");

        // GENERATE KEY PAIR
        dh.GenerateKeyPair(prng, privKey, pubKey);

        decoder.Decode(pubKey.BytePtr(), pubKey.SizeInBytes());
        ss << decoder;
        ss >> pubKeyStr;

        for(unsigned long i = 0; i < pubKeyStr.length(); i++){
            message[i] = pubKeyStr[i];
        }



        // SEND PUB KEY TO SERVER THAT IS LISTENING
        send_message(message, this->sendPort, 0);


        // CREATE SERVER FOR LISTENER'S KEY
        create_server(serverSocket, addr);


        // ACCEPT KEY
        accept_message(serverSocket,addr,receivedSocket,receivedMessage);

        // GENERATE SYMMETRIC KEY
        CryptoPP::SecByteBlock AESkey = generate_symetric_key(receivedMessage, privKey, shared, dh);


        memset(&receivedMessage[0], 0, sizeof(receivedMessage));

        // ACCEPT LISTENER'S IV
        accept_message(serverSocket,addr,receivedSocket,receivedMessage);

        for(size_t i = 0; i < strlen(receivedMessage);i++){
            iv[i] = receivedMessage[i];
        }



        memset(&receivedMessage[0], 0, sizeof(receivedMessage));
        memset(&message[0], 0, sizeof(message));



        // OPEN FILE FOR SENDING
        FILE *fp = fopen(this->fileName,"r");
        if(fp == NULL){
            std::cout << "Failed to open!" << std::endl;
            throw -1;
        }


        // GET FILE SIZE
        int size;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        std::cout << "size: " << size << std::endl;



        // SEND FILE SIZE
        sizeStr = std::to_string(size);
        for(size_t i = 0; i < sizeStr.length(); i++){
            message[i] = sizeStr[i];
        }

        send_message(message,this->sendPort,0);

        memset(&message[0], 0, sizeof(message));


        CFB_Mode<AES>::Encryption cfbEncryption(AESkey, AESkey.size(), iv);

        // SEND FILE IN SIZE OF 1024
        while(!feof(fp)) {
            int readSize = fread(message, 1, sizeof(message), fp);
            std::cout << "message: " << message <<"\n size: " << readSize << std::endl;
            cfbEncryption.ProcessData((unsigned char *)message,(unsigned char *)message,readSize);
            send_message(message,this->sendPort,readSize);
            memset(&message[0], 0, sizeof(message));
        }

        // CLOSE ALL, FINISH
        fclose(fp);
        emit text_box("File sucessfully sent!");
        close(serverSocket);
        emit finished();
    }

    catch(int e){
        emit text_box("Something went wrong, try again!");
        std::cout << "Caught an error: " << e << std::endl;
        emit finished();
    }
}
