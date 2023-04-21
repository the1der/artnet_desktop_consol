#ifndef ARTNET_SCANNER__H
#define ARTNET_SCANNER__H

#include "artnet.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <boost/asio.hpp>

#define SCAN_Period 5


using boost::asio::ip::udp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
// using namespace std;
namespace this_coro = boost::asio::this_coro;

typedef enum 
{
    searchBy_SHORT_NAME,
    searchBy_LONG_NAME,
    searchBy_IP,
    searchBy_MAC,
} searchBy_t;

struct node_t
{
//    uint8_t     ID[8];
   uint8_t     IpAddress[4];
   uint16_t    Port;
   uint8_t     ShortName[18];
   uint8_t     LongName[64];
   uint8_t     MAC[6];
   uint8_t     Subnet[4];
   bool     NodeIsDHCPCapable;
   bool     NodesIPIsDHCPConfigured;
};

struct node_str_t
{
//    uint8_t     ID[8];
   std::string     IpAddress;
   std::string     Port;
   std::string     ShortName;
   std::string     LongName;
   std::string     MAC;
   std::string     Subnet;
   bool     NodeIsDHCPCapable;
   bool     NodesIPIsDHCPConfigured;
};
class Artnet{
    private:
        static std::vector<node_t> node_vect;
    public:
        static void genArtPollRequest(SArtPoll *artPoll);
        static void genIpProgMsg(SIpProg *ipProg);
        static void megDecode(char recvBuffer[]);
        static void artPollReplyDecode(SArtPollReply *artPollReply);
        static void artIpProgDecode(SIpProgReply *ipProgReply); 
        static int nodeExists(node_t node);
        static int nodeExists(uint8_t *attribute, searchBy_t searchBy);
        static void printNodes();
        static void printNodes(std::vector<node_t> nodesVect);
        static void printNodes(std::vector<node_str_t> nodesVect);

        static awaitable<void>  timerCoro(boost::asio::io_context &io_context, udp::socket &socket);
        static awaitable<void>  artnet_broadcaster(boost::asio::io_context &io_context, udp::socket &socket, Opcode opcode);
        static awaitable<void> handle_receive(boost::asio::io_context &io_context, udp::socket &socket);
        static std::vector <node_t> runScan();
        static std::vector <node_str_t> result_to_str(std::vector<node_t> nodeVect);
        
};


#endif