#ifndef __BITBLAST_H__
#define __BITBLAST_H__

const unsigned int BITBLAST_END = 0xDEADBEEF;


/*
 * bitblaster control header.
 * NB: all items need to be in network byte order.
 */

struct bitblast_header
{
    unsigned int command;     /* 0x0 or 0xDEADBEEF */
    unsigned int sequence;
    unsigned int length;
    unsigned int send_sec;
    unsigned int send_usec;
};

const int MAXPSIZE = 1470;
const int MINPSIZE = sizeof(bitblast_header); 

union bitblast_packet
{
    bitblast_header header;
    unsigned char raw_bytes[MAXPSIZE];
};

#endif /* __BITBLAST_H__ */
