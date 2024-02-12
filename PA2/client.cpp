//Author: Edoardo Silvio Gribaldo
//Fall 2023

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h> 
#include "packet.h"

using namespace std;

int main(int argc, char *argv[]) {

    int send_sock = socket(AF_INET, SOCK_DGRAM, 0); //Handshake socket
    if (send_sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    int rec_sock = socket(AF_INET, SOCK_DGRAM, 0);  //file transfer socket
    if (rec_sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    struct hostent *server;           // pointer to a structure of type hostent
    server = gethostbyname(argv[1]);   // Gets host ip address // requires netd$
        if (server == NULL){ // failed to obtain server's name
                cout << "Failed to obtain server.\n";
                exit(EXIT_FAILURE);
        }
    int send_port = atoi(argv[2]);
    int listen_port = atoi(argv[3]);


    struct timeval tv;
    tv.tv_sec = 2;  /* 2 Secs Timeout */
    tv.tv_usec = 0;
    if (setsockopt(rec_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
        perror("setsockopt: rcvtimeo ai");
        exit(1);
    };
    // first socket

    struct sockaddr_in send_address; //server structure
    socklen_t send_address_len = sizeof(send_address);
    memset(&send_address, 0, sizeof(send_address)); //reset memory
    send_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char*)&send_address.sin_addr.s_addr, server->h_length); //copy ip address resolved above to server data structure
    //server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    send_address.sin_port = htons(send_port); //assign port number from command line




    // second socket
    struct sockaddr_in rec_address; //new server structure
    socklen_t rec_address_len = sizeof(rec_address);
    memset(&rec_address, 0, sizeof(rec_address));
    rec_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char*)&rec_address.sin_addr.s_addr, server->h_length);
    rec_address.sin_port = htons(listen_port); //assigned port from command line


    if (bind(rec_sock, (struct sockaddr *) &rec_address, sizeof(rec_address)) < 0) { //binding socket with se$
        perror("Error binding socket");
        close(rec_sock);
        close(send_sock);
        exit(1);
    }

    ifstream input_file(argv[4]); //opening file
    if (!input_file.is_open()) {
        perror("Error opening input file");
        close(send_sock);
	close(rec_sock);
        exit(1);
    }
    ofstream log_seqnum("clientseqnum.log"); //open output file
    if (!log_seqnum.is_open()) {
        perror("Error opening output file");
        close(rec_sock);
        close(send_sock);
        exit(1);
    }
    ofstream log_ack("clientack.log"); //open output file
    if (!log_ack.is_open()) {
        perror("Error opening output file");
        close(rec_sock);
        close(send_sock);
        exit(1);
    }

    int type = 1;
    int seqnum = 1;
    char data[30];
    char ack_data[10];
    char spacket[35];
    int difference= 0;
    int num_bytes;
    memset(data, 0, sizeof(data));
    while ( input_file.read(data, 30)) { //Reads 30 character from file at a time, it will read less than 4 or whatever left for the last transmission
        seqnum = !seqnum;
	data[input_file.gcount()] = '\0';
	packet data_packet(type, seqnum, sizeof(data), data);
        data_packet.serialize(spacket);
	cout << "Packet sent: " << endl; 
	data_packet.printContents();
        num_bytes = sendto(send_sock, spacket, sizeof(spacket), 0, (struct sockaddr*) &send_address, send_address_len); //sending data to server
        if (num_bytes < 0) {
	        perror("Error sending data");
        	break;
        }
	else {
		log_seqnum << data_packet.getSeqNum() << endl;
		while (recvfrom(rec_sock, ack_data, 10, 0, (struct sockaddr *)&rec_address, &rec_address_len) < 0) { //receiving data from the server
			num_bytes = sendto(send_sock, spacket, sizeof(spacket), 0, (struct sockaddr*) &send_address, send_address_len);
		}
		cout << "Sequence number of the ack packet received: "<< endl; 
		cout << ack_data[2] << endl;
		log_ack << ack_data[2] << endl;
	}
	memset(data, 0, sizeof(data));
    }

    // If we've reached the end of the file, send a final datagram with 0 bytes
    if (input_file.eof()) {
	packet fin_packet(3, !seqnum, 0, NULL);
 	fin_packet.serialize(spacket);
	fin_packet.printContents();
        num_bytes = sendto(send_sock, spacket, 33, 0, (struct sockaddr*) &send_address, send_address_len);
        if (num_bytes < 0) {
            perror("Error sending end-of-file indicator");
        }
        else {
		log_seqnum << fin_packet.getSeqNum() << endl;
        	if (recvfrom(rec_sock,ack_data, 4, 0, (struct sockaddr *)&rec_address, &rec_address_len)) {
                        cout << ack_data[2] << endl;
			log_ack << ack_data[2] << endl;
		}
        }
    }

    close(send_sock);
    close(rec_sock);
    input_file.close();
    return 0;
}

