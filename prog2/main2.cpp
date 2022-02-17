#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>

#define PORT 8080
#define SERVER_PATH "/tmp/server"

int count_length(int number);

int main(void){
	struct sockaddr_un address;
	memset(&address, 0, sizeof(address));
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, SERVER_PATH);
	
	unlink(SERVER_PATH);

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}

	int bnd = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
	if(bnd < 0){
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}

	int lstn = listen(sockfd, 1);
	if(lstn < 0){
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

	while(true){
		int new_socket = accept(sockfd, NULL, NULL);
		if(new_socket < 0){
			perror("accept() failed");
			close(sockfd);
			exit(EXIT_FAILURE);
		}

		int number;
		int r = read(new_socket, &number, sizeof(int));
		if(r!=sizeof(int)){
			std::cerr<<"Error while reading\n";
			exit(EXIT_FAILURE);
		}		
		if(count_length(number)>2 && (number%32==0)){
			std::cout<<"Data received: "<<number<<std::endl;
		}
		else{
			std::cerr<<"Received data doesn't match conditions\n";
		}
		close(new_socket);
	}
}

int count_length(int number){
	int counter = 1;
	while(number/=10){
		counter++;
	}
	return counter;
}

