#include <iostream>
#include <string>

#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

void
usage(char *program_name)
{
    std::cerr << "usage: " << program_name << " ipaddr port [string to send]" << std::endl;
    exit(0);
}


int
main(int argc, char **argv)
{
    /**
    * ./client <ip> <port> [string to send]
    */

    if (argc < 3)
    {
        usage(argv[0]);
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        exit(0);
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    // set the IP address in our struct sockaddr_in
    if (inet_pton(AF_INET, argv[1], &sin.sin_addr) <= 0) {
        std::cerr << "Couldn't convert " << argv[1] << " to a packed IP address" << std::endl;
        exit(0);
    }

    // set the port in our struct sockaddr_in
    unsigned short port = atoi(argv[2]);
    sin.sin_port = htons(port);

    std::string data = "";
    if (argc >= 4)
        data = argv[3];
   
    int rv = sendto(sock, data.c_str(), data.size(), 0, (const struct sockaddr*)&sin, sizeof(sin));
    if (rv < 0) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        exit(0);
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
            std::cout << "From " << inet_ntoa(server_sin.sin_addr) << ":"
                      << ntohs(server_sin.sin_port) << " -- "
                      << data << std::endl;
        } else {
            std::cerr << "Didn't get anything from the server.  Weird." << std::endl;
        }
    }
   
    // be a good citizen and close the socket
    close(sock);
    return 0;
}
