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
        /**
         * @brief Create ArtPollRequest message
         * 
         * @param artPoll Pointer to the buffer where the ArtPoll message will be stored.
         */
        static void genArtPollRequest(SArtPoll *artPoll);

        /**
         * @brief Create IpProgMsg message
         * @note No programming flag is enabled, it sends an enquiry message.
         * 
         * @param ipProg 
         */
        static void genIpProgMsg(SIpProg *ipProg);

        /**
         * @brief Decodes an Art-Net message and takes action based on the opcode.
         *
         * This function decodes an Art-Net message stored in the `recvBuffer` array and takes
         * action based on the opcode contained in the message. The `recvBuffer` array must
         * contain a complete Art-Net message, including the header and any optional data.
         *
         * @param recvBuffer Pointer to the buffer containing the received Art-Net message. 
         */
        static void megDecode(uint8_t recvBuffer[]);

        /**
         * @brief Decode artPollReplyDecode. And the data of the node to node_vect, it the node already exists it will update its data.
         * 
         * @param recvBuffer Pointer the recieved artPollReply buffer.
         * @note Make sute the recvBuffer is an artPollReply message, there is no cheching of the OpCode in this function.
         */
        static void artPollReplyDecode(uint8_t recvBuffer[]);

        /**
         * @brief Decode artIpProgReply message, and save node's ip configuration
         * 
         * @param ipProgReply pointer to recieved artPollReply
         */
        static void artIpProgReplyDecode(SIpProgReply *ipProgReply); 

        /**
         * @brief checks is node exists in node_vect.
         * 
         * @param node data of the node we are searching
         * @return The index of the node in the vector that matches the searched node, or -1 if
         *         no matching node is found.
         * @note This function uses MAC address as search criteria.
         */
        static int nodeExists(node_t node);

        /**
         * @brief Searches for a node in the vector of nodes based on the specified search criteria.
         *
         * This function searches for a node in the vector of nodes based on the specified search
         * criteria. The search is performed by comparing the specified value against the value of
         * the specified member of each node in the vector. The member to be compared is identified
         * by the `dataType` parameter, which should be set to one of the values in the `searchBy_t`
         * enum (searchBy_SHORT_NAME, searchBy_LONG_NAME, searchBy_IP or searchBy_MAC).
         *
         * @param searchValue Pointer to the value to search for. The type of this value must match
         *                    the type of the member being searched.
         * @param dataType The type of data to use for the search. This should be set to one of the
         *                 values in the `searchBy_t` enum.
         *
         * @return The index of the node in the vector that matches the search criteria, or -1 if
         *         no matching node is found..
         */
        static int nodeExists(uint8_t *searchValue, searchBy_t dataType);

        /**
         * @brief Prints data of the nodes stored in node_vect
         * 
         */
        static void printNodes();

        /**
         * @brief print data of nodes of provided node_t vector
         * 
         * @param nodesVect vector to be printed
         */
        static void printNodes(std::vector<node_t> nodesVect);
        static void printNodes(std::vector<node_str_t> nodesVect);

        /**
         * @brief Takes control of the scan.
         * 
         * This function controls the delays, to make sure IpProg message is sent after receiving ArtPollReply messages
         * 
         * @param io_context 
         * @param socket 
         * @return awaitable<void> 
         */
        static awaitable<void>  timerCoro(boost::asio::io_context &io_context, udp::socket &socket);

        /**
         * @brief Broadcast ArtnetPacket corresponding to given Opcode
         * 
         * @param io_context Boost asio context
         * @param socket Server socket
         * @param opcode Art-Net OpCode of command
         * @return awaitable<void> 
         */
        static awaitable<void>  artnet_broadcaster(boost::asio::io_context &io_context, udp::socket &socket, Opcode opcode);

        /**
         * @brief Handles recieve packets
         * 
         * @param io_context Boost asio context
         * @param socket Server socket
         * @return awaitable<void> 
         */
        static awaitable<void> handle_receive(boost::asio::io_context &io_context, udp::socket &socket);

        /**
         * @brief Run network scan in the Network
         * 
         * Run a scan for ART-Net nodes by Broadcasting ArtPollRequest message
         * 
         * 
         * @return std::vector <node_t> Vector of node_t  
         */
        static std::vector <node_t> runScan();

        /**
         * @brief Convert results to string 
         * 
         * @param nodeVect Vector of nodes to be converted
         * @return std::vector <node_str_t> 
         */
        static std::vector <node_str_t> result_to_str(std::vector<node_t> nodeVect);
        
};


#endif