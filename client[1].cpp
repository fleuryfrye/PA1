//Author: Charan Gudla
//Spring 2023

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h> 

//#define BUF_SIZE 4

using namespace std;

int main(int argc, char *argv[]) {

//#####################################################
//SENDING HANDSHAKE MESSAGE
//#####################################################

    int hsock = socket(AF_INET, SOCK_DGRAM, 0); //Handshake socket
    if (hsock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    struct hostent *server;           // pointer to a structure of type hostent
    server = gethostbyname(argv[1]);   // Gets host ip address // requires netdb.h 
	if(server == NULL){ // failed to obtain server's name
		cout << "Failed to obtain server.\n";
		exit(EXIT_FAILURE);
	}
    
    struct sockaddr_in server_address; //server structure
    socklen_t Server_Length = sizeof(server_address);
    memset(&server_address, 0, sizeof(server_address)); //reset memory
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char*)&server_address.sin_addr.s_addr, server->h_length); //copy ip address resolved above to server data structure
    //server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(atoi(argv[2])); //assign port number


 	// Send "ABC" message to server
    sendto(hsock, "ABC", 3, 0, (struct sockaddr *)&server_address, Server_Length); //sending message to server

    // Receive random port number from server
    char buffer[5];
    memset(buffer,0,sizeof(buffer));
    int nBytes = recvfrom(hsock, buffer, 5, 0, (struct sockaddr *)&server_address, &Server_Length); //receiving bytes from server
    buffer[nBytes] = '\0'; //null terminating the received string
    printf("Received random port number: %s\n", buffer);
    int rport = atoi(buffer); //converting to integer from string
    memset(buffer, 0, sizeof(buffer));
    // Close socket
    close(hsock);



//################################################
//FILE TRANSFER
//################################################
    int fsock = socket(AF_INET, SOCK_DGRAM, 0); //file transfer socket
    if (fsock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    struct sockaddr_in server_rand_address; //new server structure
    socklen_t Server_Rand_Length = sizeof(server_rand_address);
    memset(&server_rand_address, 0, sizeof(server_rand_address));
    server_rand_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char*)&server_rand_address.sin_addr.s_addr, server->h_length);
    //server_rand_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_rand_address.sin_port = htons(rport); //assigned received random port from server
	

    ifstream input_file(argv[3]); //opening file
    if (!input_file.is_open()) {
        perror("Error opening input file");
        close(fsock);
        exit(1);
    }

    char data[4];
    memset(data, 0, sizeof(data));
    ssize_t num_bytes;
    while (input_file.read(data, 4)) { //Reads 4 character from file at a time, it will read less than 4 or whatever left for the last transmission
	data[4] = '\0'; //null termination
    
    num_bytes = sendto(fsock, data, 4, 0, (struct sockaddr*) &server_rand_address, sizeof(server_rand_address)); //sending data to server
    if (num_bytes < 0) {
        perror("Error sending data");
        break;
    }
	recvfrom(fsock, data, 4, 0, (struct sockaddr *)&server_rand_address, &Server_Rand_Length); //receiving data from the server
	cout<<data<<endl;
	memset(data, 0, sizeof(data));
    }

    // If we've reached the end of the file, send a final datagram with 0 bytes
    if (input_file.eof()) {
        num_bytes = sendto(fsock, data, 0, 0, (struct sockaddr*) &server_rand_address, sizeof(server_rand_address));
        if (num_bytes < 0) {
            perror("Error sending end-of-file indicator");
        }
    }

    close(fsock);
    input_file.close();
    return 0;
}
