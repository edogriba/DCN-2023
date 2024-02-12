// Author: Edoardo Silvio Gribaldo eg1005

#include<iostream>
#include <fstream>       // used for file handling
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <time.h>        // used for random number generation
#include <string.h>      // using this to convert random port integer to string
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]){

  struct sockaddr_in server;
  struct sockaddr_in client;
  int mysocket = 0;
  int i = 0;
  int count = 0;
  int max = 65535;
  int min = 1024;
  socklen_t clen = sizeof(client);
  char payload[512];
  char payload2[5];
  // Creation of the first socket
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in socket creation.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(stoi(argv[1]));
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  // Binding of the first socket
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1)
    cout << "Error in binding.\n";

  // Waiting for a connnection  on the port typed in as argument
  if (recvfrom(mysocket, payload, 512, 0, (struct sockaddr *)&client, &clen)==-1)
      cout << "Failed to receive.\n";
  // Creation of the random port
  srand((unsigned) time(NULL));
  int rand_port = rand()%(max-min+1) + min;
  char port[6];
  snprintf(port, sizeof(port), "%d", rand_port);
  // Sending the random port as acknowledge request
  if (sendto(mysocket, port, 512, 0, (struct sockaddr *)&client, clen)==-1){
    cout << "Error in send to function.\n";
  }
  // Printing on the screen random port
  cout << "Random port: " << port << endl;
  // Closing of the first socket
  close(mysocket);

  // creation of the second socket
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in socket creation.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(stoi(port));
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1) {
    cout << "Error in binding.\n";
  }
  // Opening of the output file
  ofstream fout("upload.txt");
  // Waiting for chunks of the file to transfer and sending acknowledge for every chunk received
  while (recvfrom(mysocket, payload2, 5, 0, (struct sockaddr *)&client, &clen) !=-1) {
  count=0;
  fout << payload2;
  for (auto & c: payload2) {
  c = toupper(c);
  }
  if (sendto(mysocket, payload2, 5, 0, (struct sockaddr *)&client, clen)==-1){
        cout << "Error in send to function.\n";
  }
  while (payload2[count] != '\0') {
  count++;
  }
  if (count < 4) {
  break;
  }
  }
  // Closing of the file
  fout.close();
  // Closing of the second socket
  close(mysocket);
  return 0;
}
