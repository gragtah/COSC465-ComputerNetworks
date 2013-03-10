#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

void usage(char *program_name)
{
    std::cerr << "usage: " << program_name << " or " << program_name << " <username> <blat text>" << std::endl;
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc!=3 && argc!=1) 
    {
        usage(argv[0]);
    }
    const char *ipaddress = "149.43.80.25";
    const char *UDPport = "2468";

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        exit(0);
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    // set the IP address in our struct sockaddr_in
    if (inet_pton(AF_INET, ipaddress, &sin.sin_addr) <= 0) {
        std::cerr << "Couldn't convert " << argv[1] << " to a packed IP address" << std::endl;
        exit(0);
    }

    // set the port in our struct sockaddr_in
    unsigned short port = atoi(UDPport);
    sin.sin_port = htons(port);
    int rv;

    //GETBLAT request
    if(argc==1)
    {
      std::string request = "GETBLAT";
        rv = sendto(sock, request.c_str(), 7, 0, (const struct sockaddr*)&sin, sizeof(sin));
        if (rv < 0) {
            std::cerr << "Error: " << strerror(errno) << std::endl;
            exit(0);
        }
    }
    //PUTBLAT request
    else if(argc==3)
    {
        std :: ostringstream ostr;
        std::string user = argv[1];
        std::string blat = argv[2];
        std::string request = "PUTBLAT";
        ostr << request << std::left << std::setfill(' ') 
             << std::setw(10) << user
             << std::setw(50) << blat;
        std::string data = ostr.str();

        rv = sendto(sock, data.c_str(), data.size(), 0, (const struct sockaddr*)&sin, sizeof(sin));
        if (rv < 0) {
            std::cerr << "Error: " << strerror(errno) << std::endl;
            exit(0);
        }

    }

    struct pollfd pfd[1];
    pfd[0].fd = sock;
    pfd[0].events = POLLIN | POLLERR;
    pfd[0].revents = 0;

    rv = poll(pfd, 1, 1000);
    if (rv == 0) {
        std::cerr << "Poll timed out.  Server must be sleeping." << std::endl;
    } else if (rv < 0) {
        std::cerr << "Error in poll " << strerror(errno) << std::endl;
    } else if (pfd[0].revents & POLLIN) {
        struct sockaddr_in server_sin;
        socklen_t sinlen = sizeof(server_sin);
        char buffer[4096];
        memset(buffer, 0, 4096);
        rv = recvfrom(sock, buffer, 4096, 0, (struct sockaddr*)&server_sin, &sinlen);
        if (rv > 0) {
            std::string data;
            data.assign(buffer, rv);
            std::string response = data.substr(0,2);
            std::cout << "Response len: " << rv << ".\t"<< "Response status: "<< response << std::endl; 
            if(response.compare("NO")==0){
              std::cout <<"Reason: "<< data.substr(2,rv-2) << std::endl;
            }
            else if (response.compare("OK")==0 && data.length() > 2 ) { 
              std::string numblats = data.substr(2,2);
              std::cout << "Got " << numblats << " blats" << std::endl; 
              int num = atoi(numblats.c_str()); int i, currindex=4;
              for(i=0; i<num; i++){
                std::cout << (i+1) << " " << data.substr(currindex,10) << "\t: "<< data.substr(currindex+10,50)<< std::endl;
                currindex += 60;
              }

            }
        } else {
            std::cerr << "Didn't get anything from the server.  Weird." << std::endl;
        }
    }
   
    // close the socket
    close(sock);
    return 0;

}
