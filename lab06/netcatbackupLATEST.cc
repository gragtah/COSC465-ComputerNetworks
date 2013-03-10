//LAB 06
//BY CAROLINE CRAGIN, GAURAV RAGTAH, AJ GANIK

#include <iostream>
#include <sstream>
#include <string>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>

#include <stdio.h>

using namespace std;

//similar to unix utility 'nc'

void run(int sock, sockaddr_in sin, char* mode)
{
  	int aSock, flag, rv; //flag: 0=run as server, 1=as client
	socklen_t addrlen = sizeof(struct sockaddr_in);
  	if (strcmp(mode,"-s")==0) //running as server
  	{
  		flag = 0;
		//ask OS to allow program to re-bind to given port without having to wait for "2 x msl" timeout:
		int opt = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		
		//BIND:
		rv = bind(sock, (const struct sockaddr*)&sin, sizeof(sin));
		if (rv < 0) 
		{
			std::cerr << "Error1: " << strerror(errno) << std::endl;
			exit(0);
		}
		
		//LISTEN:
		rv = listen(sock, 1); //keep 1 waiting client in queue
		if (rv < 0) 
		{
			std::cerr << "Error2: " << strerror(errno) << std::endl;
			exit(0);
		}
	
		//ACCEPT: (sit waiting for client to connect-- get new socket to use to send/recv, and use this in conjunction with poll)
    	aSock = accept(sock, (struct sockaddr*)&sin, &addrlen);
		close(sock); //close previous socket
		if (aSock < 0) 
		{
			std::cerr << "Error3: " << strerror(errno) << std::endl;
			exit(0);
		}
	}
	else //run as client
	{
        flag = 1;
        char* host = mode;
        //set IP address
        if (inet_pton(AF_INET, host, &sin.sin_addr) <= 0) 
		{
        	std::cerr << "Couldn't convert " << host << " to a packed IP address" << std::endl;
        	exit(0);
    	}
        rv = connect(sock, (struct sockaddr*)&sin, addrlen);
        if (rv < 0) 
		{
			std::cerr << "Error4: " << strerror(errno) << std::endl;
			exit(0);
		}
		aSock = sock; //want to be able to use the same socket variable in polling below, but the actual socket descriptor doesn't change when running as client
	}
	//poll:
	int valid=1, timeout = 10000; //eofflag = 0;
    while (valid) 
    {
		struct pollfd pfd[2];
		pfd[0].fd = aSock; //test for data from the network
		pfd[0].events = POLLIN | POLLERR | POLLPRI;
		pfd[0].revents = 0;
		pfd[1].fd = 0; //test for data from stdin
		pfd[1].events = POLLIN | POLLERR;
		pfd[1].revents = 0;
		
		int rv = poll(pfd, 2, timeout); //10000 milliseconds=10 seconds we want to wait before timing out
		if (rv == 0)
                {
                  if(timeout > 1500) { std::cerr << "Poll timed out." << std::endl;}
                  else { std::cerr << "Goodbye." << std::endl;}
                  if (flag) {
                      close(aSock); //sock?
                      exit(0);
                  }

                }
		else if (rv < 0) {std::cerr << "Error in poll " << strerror(errno) << std::endl;}
		else if (pfd[0].revents || pfd[1].revents)  
		{
			if (pfd[0].revents) //data coming network
			{
				char buffer[4096];
				memset(buffer, 0, 4096);
				rv = recv(aSock, buffer, 4096, 0);
				if (rv > 0) 
				{
					std::string data;
					data.assign(buffer, rv);
					std::cout << "From " << inet_ntoa(sin.sin_addr) << ":"
							  << ntohs(sin.sin_port) << " -- "
							  << data << std::endl;
				} 
				else if (rv ==0)
				{
					//valid = false; //socket is closed
					//if (flag) //client
					  // {	
                                             exit(0);
                                          // }
				}
				else
				{
					std::cerr << "Didn't get anything from the client.  Weird." << std::endl;
				}
			}
			if (pfd[1].revents) //data coming from stdin
			{
				std::string buffer;
				if(!std::cin.eof())
				{
					getline(std::cin, buffer); //we would have liked to add line breaks, but the results were too varied
					const char* sendBuffer = buffer.c_str(); //for sending, buffer needs to be char array
					unsigned int dataSent = send(aSock, sendBuffer, strlen(sendBuffer), 0);
					while (dataSent < strlen(sendBuffer))
					{
						dataSent = send(aSock, &sendBuffer[dataSent], strlen(sendBuffer) - dataSent, 0);
					}
					if (std::cin.eof()) //in case we've read the last line, exit now.
					{
						//valid = false; //EOF
						if (flag) //client
						{
                                                     std::cout <<"Receiving any remaining data on socket..."<<std::endl;
							//close(aSock);
		                		     timeout = 800;//exit(0);
						}
					}
				}
                                else if (flag)    //CHECK NEEDED client??
                                {
                                    std::cerr <<"timing out for ctrlD"<<std::endl;
                                    if (flag) timeout=800;
                                    rv = 1;
                                    while (rv)
                                    {
                                                
                                        if (pfd[0].revents) //data coming network
                                        {
				                char buffer[4096];
                                                memset(buffer, 0, 4096);
                                                rv = recv(aSock, buffer, 4096, 0);
                                                if (rv > 0) 
                                                {
                                                        std::string data;
                                                        data.assign(buffer, rv);
                                                        std::cout << "From " << inet_ntoa(sin.sin_addr) << ":"
                                                                          << ntohs(sin.sin_port) << " -- "
                                                                          << data << std::endl;
                                                } 
                                                else { exit(0);}
                                        }






                                           rv = poll(pfd, 2, timeout); 
                                     }

                                     std::cerr << "Goodbye." << std::endl;
                                     close(aSock); //sock?
                                     exit(0);
			      }
                        }
		}
		else
			std::cerr << "Didn't get anything from the client.  Weird." << std::endl;
	} //main while loop end
}

int main(int argc, char **argv)
{
	if (argc<3)
	{
		std::cerr << "Invalid arguments (less than 3)" << std::endl;
		exit(0);
	}
	else if (argc>3)
	{
		std::cerr << "Invalid arguments (more than 3)" << std::endl;
		exit(0);
	}
	else 
	{	//CREATE SOCKET & initialize sockaddr_in
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) 
		{
        	std::cerr << "Error: " << strerror(errno) << std::endl;
        	exit(0);
    	}
		struct sockaddr_in sin;
    	memset(&sin, 0, sizeof(sin));
    	sin.sin_family = AF_INET;
		unsigned short port = atoi(argv[2]);
    	sin.sin_port = htons(port);
		run(sock, sin, argv[1]);
		close(sock);
	}
    return 0;
}
