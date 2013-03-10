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

#include "bitblast.h"

#include <vector>
#include <algorithm>

struct BitBlastStat
{
    BitBlastStat() : seq(0), len(0)
        {
            memset(&send_time, 0, sizeof(send_time));
            memset(&recv_time, 0, sizeof(recv_time));
        }

    BitBlastStat(int seq, int len, const struct timeval &s, const struct timeval &r) 
        {
            this->seq = seq;
            this->len = len;
            send_time = s;
            recv_time = r;
        }

    bool operator<(const BitBlastStat &other) const
        {
            return seq < other.seq;
        }

    unsigned int seq;
    unsigned int len;
    struct timeval send_time;
    struct timeval recv_time;
};


std::ostream &operator<<(std::ostream &os, const struct BitBlastStat &bbstat)
{
   os << "seq: " << bbstat.seq << " len: " << bbstat.len 
      << " sendtime: " << bbstat.send_time.tv_sec << '.'
      << std::setw(6) << std::setfill('0') << bbstat.send_time.tv_usec 
      << " recvtime: " << bbstat.recv_time.tv_sec << '.'
      << std::setw(6) << std::setfill('0') << bbstat.recv_time.tv_usec;
   return os;
}


void usage(char *program_name)
{
    std::cerr << "usage: " << program_name << " <options>" << std::endl;
    std::cerr << "\t-p recvport" << std::endl;
    std::cerr << "\t-s sender IP address (must be an IP address in dotted notation)" << std::endl;
    std::cerr << "\t-d enable debugging output" << std::endl;
    std::cerr << "\t-h this help message" << std::endl;
    exit(0);
}


int main(int argc, char **argv)
{
    bool debug = false;
    std::string portstr = "";
    std::string senderip = "";

    int ch;
    while ((ch = getopt(argc, argv, "p:s:hd")) != -1) {
        switch (ch) {
            case 'p':
                portstr = optarg;
                break;
            case 's':
                senderip = optarg;
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

    if (portstr == "" || senderip == "") {
        usage(argv[0]);
    }
    
    /* collect the port we should listen to from command-line argument 1 */
    char *endptr = NULL;
    unsigned short port = strtol(portstr.c_str(), &endptr, 10);
    if (endptr == portstr.c_str() || port < 1024) {
        std::cerr << "Bad port number: " << portstr << ".  Needs to be > 1024." << std::endl;
        exit(0);
    }
    
    /* collect the sender's IP address from command-line argument 2 */
    struct in_addr sip;
    if (inet_aton(senderip.c_str(), &sip) == 0) {
        std::cerr << "Error interpreting " << senderip << " as the sender IP address." << std::endl;
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
    // leave sin_addr as zeros: listen on any valid local IP address
    sin.sin_port = htons(port);

    /* set socket option to allow rebinding to the same port prior to OS timeout */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        std::cerr << "Error setting socket SO_REUSEADDR option: " << strerror(errno) << std::endl;
        exit(0);
    }

    /* bind the socket to a local port so that we can correctly receive packets */
    if (bind(sockfd, (const struct sockaddr*)&sin, sizeof(struct sockaddr_in)) < 0) {
        std::cerr << "Error in bind: " << strerror(errno) << std::endl;
        exit(0);
    }

    /* make a receive buffer and set up variables before we go into receive loop */
    union bitblast_packet packet;
    struct sockaddr_in raddr;
    socklen_t ralen = 0;
    
    /* create a vector to hold receive stats */
    std::vector<BitBlastStat> recvstats;

    int rlen = 0;

    //std::cout << "bitblast receiver starting" << std::endl;
    while (true) {
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int pollrv = poll(&pfd, 1, 1000);
        if (pollrv < 0) {
            std::cerr << "Error in poll: " << strerror(errno) << std::endl;
            continue;
        } else if (pollrv == 0) {
            // timeout
            continue;
        } 
        // if we get here, there must be something ready to be received from socket
        
        
    //std::cout << "bitblast receiver attemping to receive" << std::endl;
        ralen = sizeof(struct sockaddr_in);
        if ((rlen = recvfrom(sockfd, &packet, MAXPSIZE, 0, (struct sockaddr*)&raddr, &ralen)) < 0) {
            std::cerr << "Error in recvfrom: " << strerror(errno) << std::endl;
            continue;
        }

        if (rlen < (int)sizeof(bitblast_header)) {
            std::cerr << "Not enough bytes (only " << rlen << ") received for bitblaster header." << std::endl;
            continue;
        }

        //std::cout << "bitblast receiver progressing?!" <<inet_ntoa(raddr.sin_addr) <<" " <<inet_ntoa(sip)<< " " << raddr.sin_addr.s_addr << " " << sip.s_addr << std::endl;
        
       // if (raddr.sin_addr.s_addr == sip.s_addr) {
            /* we got a packet from our intended sender.  check packet type and update stats */
            struct timeval now;
            gettimeofday(&now, 0);
            struct timeval sendtime;
            sendtime.tv_sec = ntohl(packet.header.send_sec);
            sendtime.tv_usec = ntohl(packet.header.send_usec);
            BitBlastStat currstat(ntohl(packet.header.sequence), ntohl(packet.header.length), sendtime, now);
            recvstats.push_back(currstat);

            if (recvstats.size() % 10 == 0) {
                std::cout << "Received " << recvstats.size() << "th packet." << std::endl;
            }

            unsigned int cmd = ntohl(packet.header.command);
            //std::cout << " command was " << cmd << std::endl;
            if (cmd == BITBLAST_END) {
                break;
            } else if (cmd != BITBLAST_END && cmd != 0) {
                std::cerr << "Unrecognized control command in bitblast packet header: "
                          << std::hex << "0x" << ntohl(packet.header.command) << std::endl;
            }
     //   }
    }

    close(sockfd);

    struct timeval end;
    gettimeofday(&end, 0);
    std::cout << "bitblast receiver done at " << end.tv_sec << '.' << std::setw(6) << std::setfill('0')
              << end.tv_usec << std::endl;

    // sort the vector of BitBlastStats
    std::sort(recvstats.begin(), recvstats.end());

    int lost = 0;
    int duplicates = 0;
    int bytes = 0;
    double send_spacing = 0.0;
    double recv_spacing = 0.0;
    struct timeval sdiff, rdiff;
    
    if (debug) {
        std::cout << "Dumping header info from all received packets:" << std::endl;
    }

    typedef std::vector<BitBlastStat>::const_iterator CI;
    CI enditer = recvstats.end();

    CI prev = recvstats.begin();
    CI next = prev + 1;

    if (debug) {
        std::cout << *prev << std::endl;
    }
    bytes += prev->len + 28 + 18;

    for (; next != enditer; next++, prev++) {
        if (debug) {
            std::cout << *next << std::endl; 
        }
        
        if (next->seq - prev->seq == 0) {
            duplicates++;
        } else if ((next->seq - prev->seq) > 1) {
            lost += (next->seq - prev->seq - 1);
        }
        
        /* total number of bytes transmitted for a given packet is the
           payload length of the packet plus the length of the IP
           header (20 bytes) and UDP header (8 bytes) + Ethernet (18) */
        bytes += (next->len + 28 + 18);

        /* compare packet-to-packet timing information */
        timersub(&(next->send_time), &(prev->send_time), &sdiff);
        timersub(&(next->recv_time), &(prev->recv_time), &rdiff);
        send_spacing += (sdiff.tv_sec * 1000.0) + (sdiff.tv_usec / 1000.0);
        recv_spacing += (rdiff.tv_sec * 1000.0) + (rdiff.tv_usec / 1000.0);
    }
  
    prev = recvstats.begin();
    next = recvstats.end() - 1;
    timersub(&(next->send_time), &(prev->send_time), &sdiff);
    timersub(&(next->recv_time), &(prev->recv_time), &rdiff);
    double send_rate = (bytes * 8.0) / (sdiff.tv_sec + sdiff.tv_usec / 1000000.0);
    double recv_rate = (bytes * 8.0) / (rdiff.tv_sec + rdiff.tv_usec / 1000000.0);

    std::cout << "Bytes received: " << bytes << std::endl;
    std::cout << "Packets received: " << recvstats.size() << std::endl;
    std::cout << "Packets (apparently) sent: " << recvstats.size() + lost << std::endl;
    std::cout << "Packets lost: " << lost << std::endl;
    std::cout << "Duplicate packets: " << duplicates << std::endl;
    std::cout << "Send rate (bits/sec): " << std::setprecision(12) << send_rate << std::endl;
    std::cout << "Receive rate (bits/sec): " << std::setprecision(12) << recv_rate << std::endl;

    std::cout << "Average send spacing (milliseconds): " << send_spacing / (recvstats.size()-1) << std::endl;
    std::cout << "Average receive spacing (milliseconds): " << recv_spacing / (recvstats.size()-1) << std::endl;    
    return 0;
}
