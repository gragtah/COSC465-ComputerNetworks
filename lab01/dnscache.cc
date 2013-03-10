//source file

#include <iostream>
#include <string.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include "dnscache.hh"
#include <map>
using namespace std;

//Look up the IP address corresponding to the domain name referred
//to by the string s. Store the IP address in the reference
//variable ip. Return true on success, false otherwise.
bool DnsCache::lookup(const std::string &s, struct in_addr &ip){
  
  struct timeval begin, end, diff;
  gettimeofday(&begin, 0);
  lookupnum += 1;       //num of lookups incremented every time

  //Search in cache first
  map<std::string, struct in_addr>::iterator it;
  it = cache.find(s);
  if (it != cache.end()){
    memcpy(&ip , &it->second, sizeof(ip));
    gettimeofday(&end,0);
    timersub(&end, &begin, &diff);
    lookuptime += diff.tv_sec + (diff.tv_usec / 1000000.0);
    return true;
  }

 //If not found in cache, query the DNS. Also add to cache. 
  struct addrinfo *result = 0; // ptr to lookup results
  struct addrinfo hints; // tell getnameinfo a bit about what we're looking for
  memset(&hints , 0, sizeof(hints)); // zero out the hints structure
  hints.ai_family = AF_INET; // we're only interested in IPv4 addrs
  int error = getaddrinfo(s.c_str(), 0, &hints , &result);
  if (error == 0) {
    // use the first IP address obtained from the function call
    struct sockaddr_in *sin = (struct sockaddr_in*)(result ->ai_addr);
    // put IP address in our ipaddr struct
    memcpy(&ip , &sin->sin_addr , sizeof(ip));
    // free up the memory consumed by DNS results
    freeaddrinfo(result);
    //insert into cache
    cache[s] = ip; 
    gettimeofday(&end,0);
    timersub(&end, &begin, &diff);
    lookuptime += diff.tv_sec + (diff.tv_usec / 1000000.0);
    return true;
  } 
  else {
    gettimeofday(&end,0);
    timersub(&end, &begin, &diff);
    lookuptime += diff.tv_sec + (diff.tv_usec / 1000000.0);
    return false;
  }

}

//Invalidate any cached entries related to given IP address 
void DnsCache::invalidate(const struct in_addr &ip){
  map<std::string, struct in_addr>::iterator it;
  for ( it=cache.begin() ; it != cache.end(); it++ ){
    if(it->second.s_addr == ip.s_addr) {
      cache.erase(it);
    }
  }
}

//Invalidate any cached entries related to given domain name
void DnsCache::invalidate (const std::string &s){
  map<std::string, struct in_addr>::iterator it;
  it = cache.find(s);
  if (it != cache.end()){
    cache.erase(it);
  }
}

//Return the (double) average measured time to perform
//any type of DNS lookup (cache hit AND DNS queries) in seconds
double DnsCache::average_lookup(){
  if(lookupnum==0) return 0;
  return (lookuptime/lookupnum);
}

//Print the entire contents of cache to standard output
void DnsCache::dump_cache(){
  map<std::string, struct in_addr>::iterator it;
  for ( it=cache.begin() ; it != cache.end(); it++ ){
    cout << it->first<< " => " << inet_ntoa(it->second) << endl;
  }
}

//Simple constructor
DnsCache::DnsCache(){
  lookupnum = 0;
  lookuptime = 0;
}
