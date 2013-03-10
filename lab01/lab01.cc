#include <iostream>
#include <string>
#include <vector>

#include <arpa/inet.h>
#include <sys/time.h>

#include "dnscache.hh"


void 
lookup_all(const std::vector<std::string> &hostnames, DnsCache *cache)
{
    for (std::vector<std::string>::const_iterator ci = hostnames.begin();
         ci != hostnames.end();
         ci++) {
        struct in_addr ip;
        if (cache->lookup(*ci, ip)) {
            char buffer[32];
            inet_ntop(AF_INET, &ip, buffer, 32);
            std::cout << "lookup for " << *ci << " -> " << buffer << std::endl;
        } else {
            std::cout << "lookup for " << *ci << " -> failed " << std::endl;
        }
    }
}

void
invalidate_ipstr(DnsCache *cache, const char *ipstr)
{
    struct in_addr ip;
    inet_pton(AF_INET, ipstr, &ip);
    cache->invalidate(ip);
}


int
main()
{
    DnsCache *cache = new DnsCache();

    std::vector<std::string> hostnames;
    hostnames.push_back("www.yahoo.com");
    hostnames.push_back("www.google.com");
    hostnames.push_back("www.baidu.cn");
    hostnames.push_back("smtp.google.com");
    hostnames.push_back("example.com");
    hostnames.push_back("not.a.valid.name");
    hostnames.push_back("cs.colgate.edu");

    hostnames.push_back("www.google.com");

    lookup_all(hostnames, cache);
    cache->dump_cache();
    std::cout << "Avg lookup time (none from cache): " << cache->average_lookup() << std::endl;

    lookup_all(hostnames, cache);
    cache->dump_cache();
    std::cout << "Avg lookup time (all should be from cache): " << cache->average_lookup() << std::endl;

    cache->invalidate("example.com");
    invalidate_ipstr(cache, "149.43.80.13");
     
    std::cout << "Cache contents should not contain cs.colgate.edu or example.com" << std::endl;
    cache->dump_cache();

    delete cache;
    return 0;
}

