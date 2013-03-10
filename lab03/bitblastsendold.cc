
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <poll.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>

#include "bitblast.h"

#include <vector>
#include <algorithm>

void usage(char *program_name)
{
    std::cerr << "usage: " << program_name << " <options>" << std::endl;
    std::cerr << "\t-t port" << std::endl;
    std::cerr << "\t-r target rate" << std::endl;
    std::cerr << "\t-n number of packets" << std::endl;
    std::cerr << "\t-p packet size" << std::endl;
    std::cerr << "\t-i receiver IP address (must be an IP address in dotted notation)" << std::endl;
    std::cerr << "\t-d enable debugging output" << std::endl;
    std::cerr << "\t-h this help message" << std::endl;
    exit(0);
}


int main(int argc, char **argv)
{
    bool debug = false;
    std::string portstr = "";
    std::string recvrip = "";
    int num_packets = 0;
    int target_rate = 0;
    int packet_size = 0;

    int ch;
    while ((ch = getopt(argc, argv, "i:p:t:r:n:hd")) != -1) {
        switch (ch) {
            case 'i':
                recvrip = optarg;
                break;
            case 't':
                portstr = optarg;
                break;
            case 'r':
		target_rate = atoi(optarg);
                break;
            case 'n':
		num_packets = atoi(optarg);	
                break;
            case 'p':
		packet_size = atoi(optarg);	
                break;
            case 'd':
                debug = true;
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    if (portstr == "" || recvrip == "" ||num_packets < 10 || target_rate <= 0 || packet_size<20 || packet_size >1470) {
        usage(argv[0]);
    }
    
    /* collect the port we should listen to from command-line argument 1 */
    char *endptr = NULL;
    unsigned short port = strtol(portstr.c_str(), &endptr, 10);
    if (endptr == portstr.c_str() || port < 1024 || port > 65535) {
        std::cerr << "Bad port number: " << portstr << ".  Needs to be > 1024 and < 65535." << std::endl;
        exit(0);
    }
    

    /* make a socket */
    int sockfd; 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
	exit(0);
    }	

    /* prep the sockaddr_in */
    struct sockaddr_in sin;    
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;

    // set the IP address  
    if (inet_aton(recvrip.c_str(), &sin.sin_addr) <= 0) { 
        std::cerr << "Error interpreting " << recvrip << " as the sender IP address." << std::endl;
        exit(0); 
    } 
 
    sin.sin_port = htons(port);

    /* set socket option to allow rebinding to the same port prior to OS timeout */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        std::cerr << "Error setting socket SO_REUSEADDR option: " << strerror(errno) << std::endl;
        exit(0);
    }


        //time i should wait before sending the next one 
        //     ( (packet_size * num_packets * 8) - (bit_rate * TIMEELAPSED) )/(bit_rate * .975 * (num_packets-PACKETSSENT))
    //send packets
    int i;
    int rv=0;
    struct timeval send_time;
    int groupsize=num_packets/10;

    //total time in milliseconds
    double total_time = (double) ((packet_size+46)*num_packets*8*1000) / (double) target_rate ;
    std::cout << "total time in milliseconds: "<< total_time << std::endl;
    double wait_time = (double) total_time/ (double) (num_packets -1 ); 
    std::cout << "avg wait time in milliseconds: "<< wait_time << std::endl; //num_packets - 1?
    struct timeval start_time;
    gettimeofday(&start_time, 0);
    union bitblast_packet packet;
    double wait_timex =0;
    int j=0;
    for(i=0; i<num_packets; i++){
          
        //command
        if (i< num_packets-2) { 
          packet.header.command = 0;
        }
        else {
          packet.header.command = htonl(BITBLAST_END);
         }
        //sequence
         packet.header.sequence = htonl(i);
         //length
         packet.header.length = htonl(packet_size);

         gettimeofday(&send_time, 0);
         //send_sec
         packet.header.send_sec = htonl(send_time.tv_sec);
         //send_usec 
         packet.header.send_usec = htonl(send_time.tv_usec);

         //GOTTA ADD BOTH tv_sec AND tv_usec!!
        //timersub(&end, &begin, &diff);
//    std::cout << "bitblast trying to send packet"<<(i+1) << std::endl;
            
          usleep(wait_timex);
        rv = sendto(sockfd, packet.raw_bytes, (20+packet_size), 0, (struct sockaddr*)&sin, sizeof(sin));

        if (rv < 0) {
            std::cerr << "Error: " << strerror(errno) << std::endl;
            exit(0);
        }
        if (i < num_packets - 1) {
        if (i % 2==0) {
         wait_timex = (double) 0.60* ((total_time*1000) - send_time.tv_usec + (start_time.tv_sec - send_time.tv_sec)*1000000 + start_time.tv_usec)/ (double) (num_packets -1-i );//;CHANGE HERE 
         if (wait_timex<=0) {wait_timex = 0.0;}
        }}

    //std::cout << (total_time*1000)<< " "<< send_time.tv_usec<<" "<<start_time.tv_usec << " " <<(double) ((total_time*1000) - send_time.tv_usec + (start_time.tv_sec - send_time.tv_sec)*1000000 + start_time.tv_usec)<<" & waittime: " << wait_timex<<" "<<i << " &packetsLeft: " <<(double) (num_packets -1-i ) << " " << start_time.tv_usec << " " <<send_time.tv_usec<<std::endl;

        //}
    }
    std::cout << "bitblast finally sent packet"<<i << std::endl;

    close(sockfd);

    /*
    if (debug) {
        std::cout << "Dumping header info from all received packets:" << std::endl;
    }
    */


    return 0;
}
