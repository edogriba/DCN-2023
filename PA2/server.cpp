//Author: Edoardo Silvio Gribaldo
//Fall 2023

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h> 
#include "packet.h"
 

#define BUF_SIZE 30

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
	close(rec_sock);
        exit(1);
    }

    struct hostent *server;           // pointer to a structure of type hostent
    server = gethostbyname(argv[1]);   // Gets host ip address // requires netd$
    if(server == NULL){ // failed to obtain server's name
                cout << "Failed to obtain server.\n";
                exit(EXIT_FAILURE);
    }


    int send_port = atoi(argv[3]);
    int listen_port = atoi(argv[2]);
    
    
    // first socket
    struct sockaddr_in send_address; //server structure
    socklen_t send_address_len = sizeof(send_address);
    memset(&send_address, 0, sizeof(send_address)); //reset memory
    send_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char*)&send_address.sin_addr.s_addr, server->h_length); //copy ip address resolved above to server data structure
    send_address.sin_port = htons(send_port); //assign port number




    // second socket
    struct sockaddr_in rec_address; // new server structure
    socklen_t rec_address_len = sizeof(rec_address);
    memset(&rec_address, 0, sizeof(rec_address));
    rec_address.sin_family = AF_INET;
    rec_address.sin_addr.s_addr = htonl(INADDR_ANY);
    rec_address.sin_port = htons(listen_port); //assigned received random port from server

    
    // bind
    if (bind(rec_sock, (struct sockaddr *) &rec_address, sizeof(rec_address)) < 0) { //binding socket with server receiving port
        perror("Error binding socket");
        close(rec_sock);
	close(send_sock);
        exit(1);
    }

    ofstream output_file(argv[4]); // open output file
    if (!output_file.is_open()) {
        perror("Error opening output file");
        close(rec_sock);
	close(send_sock);
        exit(1);
    }

    char buf[BUF_SIZE];
    ssize_t num_bytes;
    char data_array[30];
    char spacket_ack[6];
    int expected_seqnum = 1;
    while ((num_bytes = recvfrom(rec_sock, buf, BUF_SIZE, 0, (struct sockaddr*) &rec_address, &rec_address_len)) > 0) { //receive data from client 4 chars at a time
        expected_seqnum = !expected_seqnum;
	buf[num_bytes] = '\0';
        packet rec_packet(0,0,0, data_array);
	rec_packet.deserialize(buf);
        //check if the received packet has the expected sequence number 
        if(rec_packet.getSeqNum() == expected_seqnum){
		cout << "Received packet: " << endl;
		rec_packet.printContents();
		if (rec_packet.getType() == 3) {
			cout << "Fin packet received" << endl;
		}
                else if (output_file.is_open()) {  // Write the data to the file
                        output_file << rec_packet.getData();
                        cout << "Data written to file.\n" << endl;
                }
		else {
        		cerr << "Error opening the file for writing." << endl;
   		}
		packet ack_packet(2, !rec_packet.getSeqNum(), 0, NULL);
		cout << "Ack packet sent: " << endl;
	        ack_packet.printContents();
		ack_packet.serialize(spacket_ack);
        	sendto(send_sock, spacket_ack, sizeof(spacket_ack), 0, (struct sockaddr *)&send_address, sizeof(send_address)); //sending upper case converted data to the client as ack
        }
	else{
            cout << "Received out-of-sequence packet. Skipping..." << endl;
        }
        // Check if we've reached the end of the file
        if (rec_packet.getType() == 3) { // received 0 bytes
            break;
        }
    }

    if (num_bytes < 0) {
        perror("Error receiving data");
    }

    close(send_sock);
    close(rec_sock);
    output_file.close();
    return 0;
}
