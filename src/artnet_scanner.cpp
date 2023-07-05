#include "artnet_scanner.h"

#include "QDebug"
#include <boost/system/system_error.hpp>
#include <boost/throw_exception.hpp>
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
    qDebug() << recvBuffer ;
    if (memcmp(recvBuffer, ARTNET_ID, 8) != 0)
    {
        qDebug() << "Not ArtNet packet" ;
        return;
    }
    uint16_t msgOpCode= *(uint16_t*)(recvBuffer+8);
    qDebug() << "Recieved OpCode: " << msgOpCode ;
    switch(msgOpCode)
    {
        case OpPoll:
            qDebug() << "OpPoll request (LoopBack)" ;
            break;

        case OpPollReply:
            {
                qDebug() << "OpPollReply" ;
                artPollReplyDecode(recvBuffer);
            }
            break;

        case OpIpProg:
            qDebug() << "OpIpProg (LoopBack)" ;
            break;

        case OpIpProgReply:
            {
                qDebug() << "OpIpProgReply" ;
                SIpProgReply *ipProgReply= (SIpProgReply*)recvBuffer;
                artIpProgReplyDecode(ipProgReply);
            }
            break;

        default:
            qDebug() << "Unhandled OpCode" ;
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
        if (recvBuffer[i]==210) qDebug() << "Status location: "  << i ;
    for (int i=0; i<10; i++)
    {
    sprintf(smth, "%u, %d\n", recvBuffer[210+i], i);
    qDebug() << "ss " << smth ;
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
        qDebug() << "New node found "  << unsigned(newNode.IpAddress[0]) << unsigned(newNode.IpAddress[1]) << unsigned(newNode.IpAddress[2]) << unsigned(newNode.IpAddress[3]) << " [" << newNode.LongName << "]" ;
        sprintf(deb,"New node found, %u.%u.%u.%u [%s]", newNode.IpAddress[0], newNode.IpAddress[1], newNode.IpAddress[2], newNode.IpAddress[3], newNode.LongName);
        qDebug() << deb ;
    }
    else
    { 
        node_vect[nodePos]=newNode;
        qDebug() << "Existing node" ;
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

int Artnet::nodeExists(uint8_t *attribute, searchBy_t searchBy)  //add check size
{
    if(node_vect.size()==0) return -1;
    switch(searchBy)
    {
        case searchBy_IP:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].IpAddress, attribute, 4) == 0) return i;
            }
            break;
        
        case searchBy_LONG_NAME:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].LongName, attribute, 64) == 0) return i;
            }
            break;
            
        case searchBy_SHORT_NAME:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].ShortName, attribute, 18) == 0) return i;
            }
            break;

        case searchBy_MAC:
            for (int i=0; i<node_vect.size(); i++)
            {
                if (memcmp(node_vect[i].MAC, attribute, 6) == 0) return i;
            }
            break;
    }
    return -1;
}

void Artnet::printNodes()
{
    char ip[20], nm[20], mac[25];
    qDebug() << "Nodes List:" ;
    if (node_vect.size()==0) return;

    for(int i=0; i<node_vect.size(); i++)
    {
        sprintf(ip, "%u.%u.%u.%u", node_vect[i].IpAddress[0], node_vect[i].IpAddress[1], node_vect[i].IpAddress[2], node_vect[i].IpAddress[3]);
        sprintf(nm, "%u.%u.%u.%u", node_vect[i].Subnet[0], node_vect[i].Subnet[1], node_vect[i].Subnet[2], node_vect[i].Subnet[3]);
        sprintf(mac, "%x.%x.%x.%x.%x.%x", node_vect[i].MAC[0], node_vect[i].MAC[1], node_vect[i].MAC[2], node_vect[i].MAC[3], node_vect[i].MAC[4], node_vect[i].MAC[5]);

        qDebug() << ip << "[ " << node_vect[i].ShortName << " ] ---------------" ;
        qDebug() << "netmask: " << nm ;
        qDebug() << "MAC: " << mac ;
        qDebug() << "Long name: " << node_vect[i].LongName ;
        qDebug() << "DHCP capable: " << node_vect[i].NodeIsDHCPCapable ;
        qDebug() << "    DHCP enabled: " << node_vect[i].NodesIPIsDHCPConfigured  ;
    }
}

void Artnet::printNodes(std::vector<node_t> nodesVect)
{
    char ip[20], nm[20], mac[25];
    qDebug() << "Nodes List:" ;
    if (nodesVect.size()==0) return;

    for(int i=0; i<nodesVect.size(); i++)
    {
        sprintf(ip, "%u.%u.%u.%u", nodesVect[i].IpAddress[0], nodesVect[i].IpAddress[1], nodesVect[i].IpAddress[2], nodesVect[i].IpAddress[3]);
        sprintf(nm, "%u.%u.%u.%u", nodesVect[i].Subnet[0], nodesVect[i].Subnet[1], nodesVect[i].Subnet[2], nodesVect[i].Subnet[3]);
        sprintf(mac, "%x.%x.%x.%x.%x.%x", nodesVect[i].MAC[0], nodesVect[i].MAC[1], nodesVect[i].MAC[2], nodesVect[i].MAC[3], nodesVect[i].MAC[4], nodesVect[i].MAC[5]);

        qDebug() << ip << "[ " << nodesVect[i].ShortName << " ] ---------------" ;
        qDebug() << "netmask: " << nm ;
        qDebug() << "MAC: " << mac ;
        qDebug() << "Long name: " << nodesVect[i].LongName ;
        qDebug() << "DHCP capable: " << nodesVect[i].NodeIsDHCPCapable ;
        qDebug() << "DHCP enabled: " << nodesVect[i].NodesIPIsDHCPConfigured  ;
         
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

    qDebug() << "Received " << recv_size << " bytes from "<< sender_end.address().to_string() ;
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
  qDebug() << "Local" ;
  #else 
  boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address_v4::broadcast(), 6454);
  #endif
//   boost::asio::ip::udp::endpoint remote_endpoint(boost::asio::ip::address::from_string("192.168.32.255"), 6454);


  switch(opcode)
  {
    case OpPoll:
      {
        qDebug() << "Sending OpPoll request" ;
        uint8_t transmitBuffer[sizeof(SArtPoll)];
        SArtPoll rst;
        Artnet::genArtPollRequest(&rst);
        memcpy(transmitBuffer, &rst, sizeof(SArtPoll));
        co_await socket.async_send_to(boost::asio::buffer(transmitBuffer), remote_endpoint, use_awaitable);
      }
      break;

    case OpIpProg:
      {
        qDebug() << "Sending IpProg request" ;
        uint8_t transmitBuffer[sizeof(SIpProg)]={};
        SIpProg msg;
        Artnet::genIpProgMsg(&msg);
        memcpy(transmitBuffer, &msg, sizeof(SArtPoll));
        co_await socket.async_send_to(boost::asio::buffer(transmitBuffer), remote_endpoint, use_awaitable);
      }
      break;

    default:
      qDebug() << "Unhandled opcode" ;
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



void Artnet::genArtAddressPacket(ArtAddressPacket artAddressPacket, uint8_t *buff)
{
    uint8_t subSwitch, netSwitch;
    uint8_t swIn[4], swOut[4];
    uint8_t ledCmd;
    uint16_t ver=63488;
    uint16_t opcode = OpAddress;
    // memcpy(&netSwitch, buff + 12, 1);   
    // memcpy(shortName, buff + 14, 18);   
    // memcpy(longName, buff + 32, 64);   
    // memcpy(swIn, buff + 96, 4);   
    // memcpy(swOut, buff + 100, 4);   
    // memcpy(&subSwitch, buff + 104, 1);   
    // memcpy(&cmd, buff + 106, 1);
    memcpy(buff, ARTNET_ID, 8);
    memcpy(buff + 8, &opcode, 2);
    memcpy(buff + 10, &ver, 2);

    if(strlen(artAddressPacket.netSwitch) > 0)
    {
        netSwitch = std::stoi(std::string(artAddressPacket.netSwitch));
    }
    else
    {
        netSwitch = 0;
    }
    memcpy(buff + 12, &netSwitch, 1);  

    if(strlen(artAddressPacket.shortName) > 0)
    {
        memcpy(buff + 14, artAddressPacket.shortName, 18);  
    }

    if(strlen(artAddressPacket.longName) > 0)
    {
        memcpy(buff + 32, artAddressPacket.longName, 64);  
    }

    if(strlen(artAddressPacket.subSwitch) > 0)
    {
        subSwitch = std::stoi(std::string(artAddressPacket.subSwitch));
    }
    else 
    {
        subSwitch = 0;
    }
    memcpy(buff + 104, &subSwitch, 1);  

    for(int i = 0; i < 4; i++)
    {
        if(strlen(artAddressPacket.swIn[i]) > 0)
        {
            swIn[i] = std::stoi(std::string(artAddressPacket.swIn[i]));
        }
        else 
        {
            swIn[i] = 0;
        }
    }
    memcpy(buff + 96, &swIn, 4); 

    for(int i = 0; i < 4; i++)
    {
        if(strlen(artAddressPacket.swIn[i]) > 0)
        {
            swOut[i] = std::stoi(std::string(artAddressPacket.swOut[i]));
        }
        else 
        {
            swOut[i] = 0;
        }
    }
    memcpy(buff + 100, &swOut, 4);  

    switch( artAddressPacket.led_cmd)
    {
        case NO_CMD:
            ledCmd = 0x00;
            break;

        case LED_NORMAL:
            ledCmd = 0x02;
            break;

        case LED_MUTE:
            ledCmd = 0x03;
            break;

        case LED_LOCATE:
            ledCmd = 0x04;
            break;

        default:
            ledCmd = 0x00;
            break;
    }
    memcpy(buff + 106, &ledCmd, 1);
}

int Artnet::sendArtAdress(node_t node, ArtAddressPacket artAddressPacket)
{
    uint8_t tx_buff[128] = {0};
    genArtAddressPacket(artAddressPacket, tx_buff);

    boost::asio::io_context io_context;
    boost::asio::ip::udp::socket socket(io_context, udp::endpoint(udp::v4(), 6454));
    boost::asio::ip::address_v4 node_ip({node.IpAddress[0], node.IpAddress[1], node.IpAddress[2], node.IpAddress[3]});
    std::cout << node_ip.to_string() << std::endl;
    boost::asio::ip::udp::endpoint remote_endpoint(node_ip, 6454); //  change ip

    size_t bytes_sent = socket.send_to(boost::asio::buffer(tx_buff), remote_endpoint);
    std::cout << bytes_sent << std::endl;
    try {
        socket.close();

    } catch (boost::wrapexcept<boost::system::system_error> e) {
        std::cout << "caught\n";
    }
    try {
        socket.cancel();

    } catch (boost::wrapexcept<boost::system::system_error> e) {
        std::cout << "caught\n";
    }
    std::cout << "test" << "\n";
    if( bytes_sent < 0) 
    {
        std::cout << "No bytes sent" << std::endl;
        return -1;
    }

    else return 0;
}

void Artnet::genOpIpProgPacket(OpIpProgPacket opIpProgPacket, uint8_t *buff)
{
    uint16_t ver=63488;
    uint16_t opcode = OpIpProg;
    uint8_t ip[4];
    uint8_t netMask[4];
    uint8_t gateWay[4];
    uint16_t port;
    uint8_t cmd = 0;
    bool progIp = 1, progNetMask = 1, progGateWay = 1;

    memcpy(buff, ARTNET_ID, 8);
    memcpy(buff + 8, &opcode, 2);
    memcpy(buff + 10, &ver, 2);
    for(int i = 0; i < 4; i++)
    {
        if(strlen(opIpProgPacket.ip[i]) > 0)
        {
            ip[i] = std::stoi(std::string(opIpProgPacket.ip[i]));
        }
        else 
        {
            memset(ip, 0, 4);
            progIp = false;
            break;
        }
    }

    for(int i = 0; i < 4; i++)
    {
        if(strlen(opIpProgPacket.netMask[i]) > 0)
        {
            netMask[i] = std::stoi(std::string(opIpProgPacket.netMask[i]));
        }
        else 
        {
            memset(netMask, 0, 4);  
            progNetMask = false;
            break;
        }
    }

    for(int i = 0; i < 4; i++)
    {
        if(strlen(opIpProgPacket.gateWay[i]) > 0)
        {
            gateWay[i] = std::stoi(std::string(opIpProgPacket.gateWay[i]));
        }
        else 
        {
            memset(gateWay, 0, 4);
            progGateWay = false;
            break;
        }
    }

    cmd |= (progNetMask << 1);
    cmd |= (opIpProgPacket.resetConig << 3);
    cmd |= (progIp << 2);
    cmd |= (progGateWay << 4);
    cmd |= (opIpProgPacket.enDHCP << 6);
    cmd |= (1 << 7);

    memcpy(buff + 14, &cmd, 1);
    memcpy(buff + 16, ip, 4);
    memcpy(buff + 20, netMask, 4);
    memcpy(buff + 26, gateWay, 4);
}


int Artnet::sendOpIpProg(node_t node, OpIpProgPacket opIpProgPacket)
{
    uint8_t tx_buff[128] = {0};
    genOpIpProgPacket(opIpProgPacket, tx_buff);

    boost::asio::io_context io_context;
    boost::asio::ip::udp::socket socket(io_context, udp::endpoint(udp::v4(), 6454));
    boost::asio::ip::address_v4 node_ip({node.IpAddress[0], node.IpAddress[1], node.IpAddress[2], node.IpAddress[3]});
    std::cout << node_ip.to_string() << std::endl;
    boost::asio::ip::udp::endpoint remote_endpoint(node_ip, 6454); //  change ip

    size_t bytes_sent = socket.send_to(boost::asio::buffer(tx_buff), remote_endpoint);
    std::cout << bytes_sent << std::endl;
    try {
        socket.close();

    } catch (boost::wrapexcept<boost::system::system_error> e) {
        std::cout << "caught\n";
    }
    try {
        socket.cancel();

    } catch (boost::wrapexcept<boost::system::system_error> e) {
        std::cout << "caught\n";
    }
    std::cout << "test" << "\n";
    if( bytes_sent < 0) 
    {
        std::cout << "No bytes sent" << std::endl;
        return -1;
    }

    else return 0;
}

void Artnet::printNodes(std::vector<node_str_t> nodesVect)
{
    if (nodesVect.size()>0)
    for (int i=0; i<nodesVect.size(); i++)
    {
        qDebug() << QString::fromStdString(nodesVect[i].IpAddress) << ":" << QString::fromStdString(nodesVect[i].Port) << "[ " << QString::fromStdString(nodesVect[i].ShortName) << " ]" << "_______" ;
        qDebug() << QString::fromStdString(nodesVect[i].Subnet) ;
        qDebug() << QString::fromStdString(nodesVect[i].MAC) ;
        qDebug() << QString::fromStdString(nodesVect[i].LongName);
    }
}