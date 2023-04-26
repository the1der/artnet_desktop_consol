#include "artnet_scanner.h"

std::vector<node_t> Artnet::node_vect;

// #define LOCAL_TEST

void Artnet::genArtPollRequest(SArtPoll *artPoll)
{
    uint16_t ver=63488;
    memcpy(artPoll->ID, ARTNET_ID, 8);
    artPoll->OpCode = OpPoll;
    memcpy(artPoll+8, &ver, 2);
    artPoll->TalkToMe.SendDiag=1;
    artPoll->TalkToMe.SendPollOnChange=1;
    artPoll->TalkToMe.Unicast=0;
    artPoll->TalkToMe.unused1=0;
    artPoll->TalkToMe.unused2=0;
}

void Artnet::genIpProgMsg(SIpProg *ipProg)
{
    uint16_t ver=63488;
    memcpy(ipProg->ID, ARTNET_ID, 8);
    ipProg->OpCode = OpIpProg;
    memcpy(ipProg+8, &ver, 2);
    ipProg->command=0;
}



void Artnet::megDecode(uint8_t recvBuffer[])
{
    std::cout << recvBuffer << std::endl;
    if (memcmp(recvBuffer, ARTNET_ID, 8) != 0)
    {
        std::cout << "Not ArtNet packet" << std::endl;
        return;
    }
    uint16_t msgOpCode= *(uint16_t*)(recvBuffer+8);
    std::cout << "Recieved OpCode: " << std::hex << msgOpCode << std::endl;
    switch(msgOpCode)
    {
        case OpPoll:
            std::cout << "OpPoll request (LoopBack)" << std::endl;
            break;

        case OpPollReply:
            {
                std::cout << "OpPollReply" << std::endl;
                artPollReplyDecode(recvBuffer);
            }
            break;

        case OpIpProg:
            std::cout << "OpIpProg (LoopBack)" << std::endl;
            break;

        case OpIpProgReply:
            {
                std::cout << "OpIpProgReply" << std::endl;
                SIpProgReply *ipProgReply= (SIpProgReply*)recvBuffer;
                artIpProgReplyDecode(ipProgReply);
            }
            break;

        default:
            std::cout << "Unhandled OpCode" << std::endl;
            break;
    }
}

void Artnet::artPollReplyDecode(uint8_t recvBuffer[])
{
    SArtPollReply *artPollReply= (SArtPollReply*)recvBuffer;
    node_t newNode;
    memcpy(newNode.IpAddress, artPollReply->IpAddress, 4);
    memcpy(newNode.ShortName, artPollReply->ShortName, 18);
    memcpy(newNode.LongName, artPollReply->LongName, 64);
    // memcpy(newNode.MAC, artPollReply->MAC, 6);
    char smth[15];
    for(int i=0; i<6; i++)
    {
        newNode.MAC[i]=recvBuffer[i+201];
    }
    for(int i=0; i<255; i++) 
        if (recvBuffer[i]==210) std::cout << "Status location: "  << i << std::endl; 
    for (int i=0; i<10; i++)
    {
    sprintf(smth, "%u, %d\n", recvBuffer[210+i], i);
    std::cout << "ss " << smth << std::endl;
    strcpy(smth, "\0");
    }
    newNode.Port = artPollReply->Port;
    newNode.NodeIsDHCPCapable = (recvBuffer[212] & 4) >> 2;
    newNode.NodesIPIsDHCPConfigured = (recvBuffer[212] & 2) >> 1;
    // newNode.NodeIsDHCPCapable = artPollReply->Status2.NodeIsDHCPCapable;
    // newNode.NodesIPIsDHCPConfigured = artPollReply->Status2.NodesIPIsDHCPConfigured;
    int nodePos= nodeExists(newNode);
    if (nodePos < 0)
    {
        char deb[150];
        node_vect.push_back(newNode);
        // std::cout << "New node found " << std::endl << unsigned(newNode.IpAddress[0]) << unsigned(newNode.IpAddress[1]) << unsigned(newNode.IpAddress[2]) << unsigned(newNode.IpAddress[3]) << " [" << newNode.LongName << "]" <<std::endl;
        sprintf(deb,"New node found, %u.%u.%u.%u [%s]", newNode.IpAddress[0], newNode.IpAddress[1], newNode.IpAddress[2], newNode.IpAddress[3], newNode.LongName);
        std::cout << deb <<std::endl;
    }
    else
    { 
        node_vect[nodePos]=newNode;
        std::cout << "Existing node" << std::endl;
    }
}

void Artnet::artIpProgReplyDecode(SIpProgReply *ipProgReply)
{
    char ip[20];
    int pos = nodeExists(ipProgReply->Ip, searchBy_IP);
    if (pos < 0) return;

    memcpy(&(node_vect[pos].Subnet), ipProgReply->subnetMask, 4);
    // int pos= nodeExists()
}



int Artnet::nodeExists(node_t node)
{
    if(node_vect.size()==0) return -1;
    else
    {
        for (int i=0; i<node_vect.size(); i++)
        {
            if (memcmp(node_vect[i].MAC, node.MAC, 6) == 0) return i;
        }
    }
    return -1;
}

int Artnet::nodeExists(uint8_t *searchValue, searchBy_t searchBy)  //add check size
{
    if(node_vect.size()==0) return -1;
    switch(searchBy)
    {
        case searchBy_IP:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].IpAddress, searchValue, 4) == 0) return i;
            }
            break;
        
        case searchBy_LONG_NAME:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].LongName, searchValue, 64) == 0) return i;
            }
            break;
            
        case searchBy_SHORT_NAME:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].ShortName, searchValue, 18) == 0) return i;
            }
            break;

        case searchBy_MAC:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].MAC, searchValue, 6) == 0) return i;
            }
            break;
    }
    return -1;
}

void Artnet::printNodes()
{
    char ip[20], nm[20], mac[25];
    std::cout << "Nodes List:" << std::endl;
    if (node_vect.size()==0) return;

    for(int i=0; i<node_vect.size(); i++)
    {
        sprintf(ip, "%u.%u.%u.%u", node_vect[i].IpAddress[0], node_vect[i].IpAddress[1], node_vect[i].IpAddress[2], node_vect[i].IpAddress[3]);
        sprintf(nm, "%u.%u.%u.%u", node_vect[i].Subnet[0], node_vect[i].Subnet[1], node_vect[i].Subnet[2], node_vect[i].Subnet[3]);
        sprintf(mac, "%x.%x.%x.%x.%x.%x", node_vect[i].MAC[0], node_vect[i].MAC[1], node_vect[i].MAC[2], node_vect[i].MAC[3], node_vect[i].MAC[4], node_vect[i].MAC[5]);

        std::cout << ip << "[ " << node_vect[i].ShortName << " ] ---------------" << std::endl;
        std::cout << "netmask: " << nm << std::endl;
        std::cout << "MAC: " << mac << std::endl;
        std::cout << "Long name: " << node_vect[i].LongName << std::endl;
        std::cout << "DHCP capable: " << node_vect[i].NodeIsDHCPCapable << std::endl;
        std::cout << "DHCP enabled: " << node_vect[i].NodesIPIsDHCPConfigured << std::endl << std::endl;  
    }
}

void Artnet::printNodes(std::vector<node_t> nodesVect)
{
    char ip[20], nm[20], mac[25];
    std::cout << "Nodes List:" << std::endl;
    if (nodesVect.size()==0) return;

    for(int i=0; i<nodesVect.size(); i++)
    {
        sprintf(ip, "%u.%u.%u.%u", nodesVect[i].IpAddress[0], nodesVect[i].IpAddress[1], nodesVect[i].IpAddress[2], nodesVect[i].IpAddress[3]);
        sprintf(nm, "%u.%u.%u.%u", nodesVect[i].Subnet[0], nodesVect[i].Subnet[1], nodesVect[i].Subnet[2], nodesVect[i].Subnet[3]);
        sprintf(mac, "%x.%x.%x.%x.%x.%x", nodesVect[i].MAC[0], nodesVect[i].MAC[1], nodesVect[i].MAC[2], nodesVect[i].MAC[3], nodesVect[i].MAC[4], nodesVect[i].MAC[5]);

        std::cout << ip << "[ " << nodesVect[i].ShortName << " ] ---------------" << std::endl;
        std::cout << "netmask: " << nm << std::endl;
        std::cout << "MAC: " << mac << std::endl;
        std::cout << "Long name: " << nodesVect[i].LongName << std::endl;
        std::cout << "DHCP capable: " << nodesVect[i].NodeIsDHCPCapable << std::endl;
        std::cout << "DHCP enabled: " << nodesVect[i].NodesIPIsDHCPConfigured << std::endl << std::endl; 
         
    }
}


awaitable<void> Artnet::handle_receive(boost::asio::io_context &io_context, udp::socket &socket)
{
    uint8_t recv_buffer[255];
    boost::system::error_code ec;

    udp::endpoint sender_end;

    std::size_t recv_size = co_await socket.async_receive_from(boost::asio::buffer(recv_buffer), sender_end, use_awaitable);

    // Spawn another coroutine to continue receiving data
    co_spawn(io_context, handle_receive(io_context, socket), detached);

    // std::cout << "Received " << recv_size << " bytes from "<< sender_end.address().to_string() << std::endl;
    if(recv_size != 0) Artnet::megDecode(recv_buffer);

} 

awaitable<void> Artnet::artnet_broadcaster(boost::asio::io_context &io_context, udp::socket &socket, Opcode opcode)
{
  auto executor = co_await this_coro::executor;
 
  socket.set_option(boost::asio::ip::udp::socket::broadcast(true));
  socket.set_option(boost::asio::ip::multicast::enable_loopback(false));
    
  // Create an endpoint to send the data to
  #ifdef LOCAL_TEST
  boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address_v4::broadcast(), 1234);
  std::cout << "Local" << std::endl;
  #else 
  boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address_v4::broadcast(), 6454);
  #endif
//   boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address::from_string("192.168.32.255"), 6454);


  switch(opcode)
  {
    case OpPoll:
      {
        std::cout << "Sending OpPoll request" << std::endl;
        uint8_t transmitBuffer[sizeof(SArtPoll)];
        SArtPoll rst;
        Artnet::genArtPollRequest(&rst);
        memcpy(transmitBuffer, &rst, sizeof(SArtPoll));
        co_await socket.async_send_to(boost::asio::buffer(transmitBuffer), remote_endpoint, use_awaitable);
      }
      break;

    case OpIpProg:
      {
        std::cout << "Sending IpProg request" << std::endl;
        uint8_t transmitBuffer[sizeof(SIpProg)]={};
        SIpProg msg;
        Artnet::genIpProgMsg(&msg);
        memcpy(transmitBuffer, &msg, sizeof(SArtPoll));
        co_await socket.async_send_to(boost::asio::buffer(transmitBuffer), remote_endpoint, use_awaitable);
      }
      break;

    default:
      std::cout << "Unhandled opcode" << std::endl;
      break;
  }
  
}

awaitable<void>  Artnet::timerCoro(boost::asio::io_context &io_context, udp::socket &socket)
{
  boost::asio::steady_timer t(io_context, boost::asio::chrono::seconds(SCAN_Period));
  co_await t.async_wait(use_awaitable);
  socket.close();
  
  socket.open(udp::v4());
  socket.bind(udp::endpoint(udp::v4(), 6454));
  co_spawn(io_context, Artnet::handle_receive(io_context, socket),detached);
  co_spawn(io_context, Artnet::artnet_broadcaster(io_context, socket, OpIpProg), detached);

  t.expires_from_now(boost::asio::chrono::seconds(SCAN_Period));
  co_await t.async_wait(use_awaitable);
  socket.close();
  socket.cancel();
}


std::vector <node_t> Artnet::runScan()
{
    try
  {
    boost::asio::io_context io_context;
    boost::asio::ip::udp::socket socket(io_context, udp::endpoint(udp::v4(), 6454));

    co_spawn(io_context, Artnet::handle_receive(io_context, socket),detached);
    co_spawn(io_context, Artnet::artnet_broadcaster(io_context, socket, OpPoll), detached);
    co_spawn(io_context, Artnet::timerCoro(io_context, socket), detached);

    io_context.run();

    Artnet::printNodes();
    return node_vect;
  }
  catch (std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
    return {};
  }
    return {};

}

std::vector <node_str_t> Artnet::result_to_str(std::vector<node_t> nodesVect)
{
    char ip[20], nm[20], mac[25];
    std::vector <node_str_t> ret_vect;
    node_str_t ret_node;
    if (nodesVect.size()>0)
    for (int i=0; i<nodesVect.size(); i++)
    {
        sprintf(ip, "%u.%u.%u.%u\0", nodesVect[i].IpAddress[0], nodesVect[i].IpAddress[1], nodesVect[i].IpAddress[2], nodesVect[i].IpAddress[3]);
        sprintf(nm, "%u.%u.%u.%u\0", nodesVect[i].Subnet[0], nodesVect[i].Subnet[1], nodesVect[i].Subnet[2], nodesVect[i].Subnet[3]);
        sprintf(mac, "%x.%x.%x.%x.%x.%x\0", nodesVect[i].MAC[0], nodesVect[i].MAC[1], nodesVect[i].MAC[2], nodesVect[i].MAC[3], nodesVect[i].MAC[4], nodesVect[i].MAC[5]);
        ret_node.IpAddress.assign(ip);
        ret_node.Subnet.assign(nm);
        ret_node.MAC.assign(mac);
        ret_node.Port= std::to_string(nodesVect[i].Port);
        ret_node.LongName.assign((char*)nodesVect[i].LongName);
        ret_node.ShortName.assign((char*)nodesVect[i].ShortName);
        ret_node.NodeIsDHCPCapable=nodesVect[i].NodeIsDHCPCapable;
        ret_node.NodesIPIsDHCPConfigured=nodesVect[i].NodesIPIsDHCPConfigured;

        ret_vect.push_back(ret_node);
        strcpy(ip,"\0");
        strcpy(nm,"\0");
        strcpy(mac,"\0");
    }
    return ret_vect;
}

void Artnet::printNodes(std::vector<node_str_t> nodesVect)
{
    if (nodesVect.size()>0)
    for (int i=0; i<nodesVect.size(); i++)
    {
        std::cout << nodesVect[i].IpAddress << ":" << nodesVect[i].Port << "[ " << nodesVect[i].ShortName << " ]" << "_______" <<std::endl;
        std::cout << nodesVect[i].Subnet << std::endl;
        std::cout << nodesVect[i].MAC << std::endl;
        std::cout << nodesVect[i].LongName << std::endl;
    }
}