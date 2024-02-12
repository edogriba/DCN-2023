// Author: Edoardo Silvio Gribaldo eg1005

#include <iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include <arpa/inet.h>   // if you want to use inet_addr() function
#include <string.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]){

  struct hostent *s;
  s = gethostbyname(argv[2]);

  struct sockaddr_in server;
  int mysocket = 0;
  socklen_t slen = sizeof(server);
  char payload[512]="ABC";
  // Creation of the first socket
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in creating socket.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(stoi(argv[1]));
  bcopy((char *)s->h_addr,
	(char *)&server.sin_addr.s_addr,
	s->h_length);

  // Send the ABC payload to initiate the handshake
  if (sendto(mysocket, payload, 8, 0, (struct sockaddr *)&server, slen) == -1) {
      cout << "Error in send to function." << endl;
  }
  char ack[512];
  // Receive the random port as acknowledge response
  recvfrom(mysocket, ack, 512, 0, (struct sockaddr *)&server, &slen);
  // Closing of the first socket
  close(mysocket);

  // File transfer part

  char buffer[5];
  char ack2[4];
  int rem_chars = 4;
  struct hostent *s2;
  s2 = gethostbyname(argv[2]);

  struct sockaddr_in server2;
  int mysocket2 = 0;
  socklen_t slen2 = sizeof(server2);

  // Creation of the second socket
  if ((mysocket2=socket(AF_INET, SOCK_DGRAM, 0))==-1) {
    cout << "Error in creating socket.\n";
  }

  memset((char *) &server2, 0, sizeof(server2));
  server2.sin_family = AF_INET;
  server2.sin_port = htons(stoi(ack));
  bcopy((char *)s2->h_addr,
        (char *)&server2.sin_addr.s_addr,
        s2->h_length);
  // Opening of the file to transfer
   ifstream fin(argv[3], ios_base::in);
   if (!fin) {
        cout << "Error opening the file." << endl;
        return 1;
   }
  // Reading, sending and waiting for acknowledge four characters by four of the file
   while (fin.read(buffer, 4)) {
   buffer[5] = '\0';
   if (sendto(mysocket2, buffer, 5, 0, (struct sockaddr *)&server2, slen2) == -1) {
      cout << "Error in sendto function." << endl;
   }
   if (recvfrom(mysocket2, ack2, 5, 0, (struct sockaddr *)&server2, &slen2) == -1) {
   cout  << "Error in recve function" << endl;
   }
   cout << ack2 << endl;
   }
   rem_chars = fin.gcount();
   if ( rem_chars < 4)  {
   fin.read(buffer, rem_chars);
   buffer[rem_chars] = '\0';
   if (sendto(mysocket2, buffer, rem_chars+1, 0, (struct sockaddr *)&server2, slen2) == -1) {
      cout << "Error in sendto function." << endl;
   }
   if (recvfrom(mysocket2, ack2, rem_chars+1, 0, (struct sockaddr *)&server2, &slen2) == -1) {
   cout  << "Error in recve function" << endl;
   }
   cout << ack2 << endl;
   }
   // Closing of the file
   fin.close();
   // Closing of the second socket
   close(mysocket2);
   return 0;
}
