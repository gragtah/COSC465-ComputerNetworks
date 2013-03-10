//header file

#include <iostream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <map>

class DnsCache{
  private:
    std::map<std::string, struct in_addr> cache;
    double lookuptime;  //lookup time
    int lookupnum;      //num of lookups
    
  public:
    bool lookup(const std::string &, struct in_addr &);
    void invalidate(const struct in_addr &);
    void invalidate (const std::string &);
    double average_lookup();
    void dump_cache();
    DnsCache();
};
