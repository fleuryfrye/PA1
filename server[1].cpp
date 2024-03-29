
//Author: Charan Gudla
//Spring 2023



#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 4

using namespace std;

int main(int argc, char *argv[]) {


//################################################
//HAND SHAKE AND GET RANDOM PORT
//################################################
	struct sockaddr_in client_address; //client structure
	socklen_t client_address_len = sizeof(client_address);

	int hsock = socket(AF_INET, SOCK_DGRAM, 0); //Handshake socket
    if (hsock < 0) {
        perror("Error creating socket");
        exit(1);
    }


 	struct sockaddr_in server_address; //server structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1])); //assign port number

    if (bind(hsock, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) { //binding socket with server port
        perror("Error binding socket");
        close(hsock);
        exit(1);
    }


	char buffer[BUF_SIZE];
	int nBytes = recvfrom(hsock, buffer, 1024, 0, (struct sockaddr *)&client_address, &client_address_len); //receiving the data from client
    buffer[nBytes] = '\0';
    printf("Received message: %s\n", buffer);

	// generate a random port
	srand(time(NULL));
	int randPort = (rand() % 64511) + 1024;
	char rport[5];
	memset(rport, 0, sizeof(rport));
	sprintf(rport, "%d", randPort);
	cout << endl << "The random port chosen is "  << rport << endl <<  endl;

	sendto(hsock, rport, strlen(rport), 0, (struct sockaddr *)&client_address, client_address_len); //sending port to client

	memset(rport, 0, sizeof(rport));
	memset(buffer, 0, sizeof(buffer));
	// Close socket
    close(hsock);

//################################################
//FILE TRANSFER
//################################################
	int fsock = socket(AF_INET, SOCK_DGRAM, 0);  //file transfer socket
    if (fsock < 0) {
        perror("Error creating socket");
        exit(1);
    }


    struct sockaddr_in server_rand_address; //new server structure
    memset(&server_address, 0, sizeof(server_rand_address));
    server_rand_address.sin_family = AF_INET;
    server_rand_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_rand_address.sin_port = htons(randPort); //assigned generated random port

    if (bind(fsock, (struct sockaddr*) &server_rand_address, sizeof(server_rand_address)) < 0) { //binding socket with server port
        perror("Error binding socket");
        close(fsock);
        exit(1);
    }


    ofstream output_file("output.txt"); //open output file
    if (!output_file.is_open()) {
        perror("Error opening output file");
        close(fsock);
        exit(1);
    }

    char buf[BUF_SIZE];
    ssize_t num_bytes;
	char ack[4];
    while ((num_bytes = recvfrom(fsock, buf, BUF_SIZE, 0, (struct sockaddr*) &client_address, &client_address_len)) > 0) { //receive data from client 4 chars at a time
        buf[num_bytes] = '\0';
        output_file.write(buf, num_bytes); //writing the received data to file
		memset(ack, 0, 4); 
		for(int i = 0; i<4; i++){
		ack[i] = toupper(buf[i]); //converting the received data to upper case
		}

		sendto(fsock, ack, strlen(buf), 0, (struct sockaddr *)&client_address, client_address_len); //sending upper case converted data to the client as ack

        // Check if we've reached the end of the file
        if (num_bytes < BUF_SIZE) { //received 0 bytes
            break;
        }
    }

    if (num_bytes < 0) {
        perror("Error receiving data");
    }

    close(fsock);
    output_file.close();
    return 0;
}
