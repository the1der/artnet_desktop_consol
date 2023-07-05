#include <cstdio>
#include <iostream>
#include "artnet_scanner.h"

// #define ARTADDRESS_TEST
#define OPIPPROG_TEST


int main()
{

    std::vector <node_t> results;
    results = Artnet::runScan();
    std::cout << std::endl << std::endl << "Results: " << std::endl;
    Artnet::printNodes(results);
    // Artnet::printNodes(Artnet::result_to_str(results));

    #ifdef ARTADDRESS_TEST
    ArtAddressPacket artAddressPacket; 
    strcpy(artAddressPacket.longName, "new long Name");
    strcpy(artAddressPacket.shortName, "new short Name");
    strcpy(artAddressPacket.subSwitch, "49");
    strcpy(artAddressPacket.netSwitch, "104");

    strcpy(artAddressPacket.swIn[0], "12");
    strcpy(artAddressPacket.swIn[1], "13");
    strcpy(artAddressPacket.swIn[2], "14");
    strcpy(artAddressPacket.swIn[3], "15");

    strcpy(artAddressPacket.swOut[0], "22");
    strcpy(artAddressPacket.swOut[1], "23");
    strcpy(artAddressPacket.swOut[2], "24");
    strcpy(artAddressPacket.swOut[3], "25");
    artAddressPacket.led_cmd = LED_NORMAL;

    if(results.size() > 0)
    {
        std::cout << "Sending ArtAdress packet" << std::endl;
        Artnet::sendArtAdress(results[0], artAddressPacket);
    }
    else
    {
        std::cout << "No nodes to configure" << std::endl;
    }
    #endif

    #ifdef OPIPPROG_TEST
    OpIpProgPacket opIpProgPacket = {0}; 

    strcpy(opIpProgPacket.ip[0], "192");
    strcpy(opIpProgPacket.ip[1], "168");
    strcpy(opIpProgPacket.ip[2], "140");
    strcpy(opIpProgPacket.ip[3], "155");

    strcpy(opIpProgPacket.netMask[0], "255");
    strcpy(opIpProgPacket.netMask[1], "255");
    strcpy(opIpProgPacket.netMask[2], "255");
    strcpy(opIpProgPacket.netMask[3], "0");

    strcpy(opIpProgPacket.gateWay[1], "168");
    strcpy(opIpProgPacket.gateWay[0], "192");
    strcpy(opIpProgPacket.gateWay[2], "140");
    strcpy(opIpProgPacket.gateWay[3], "254");

    // strcpy(opIpProgPacket.ip[0], "");
    // strcpy(opIpProgPacket.ip[1], "");
    // strcpy(opIpProgPacket.ip[2], "");
    // strcpy(opIpProgPacket.ip[3], "");

    // strcpy(opIpProgPacket.netMask[0], "");
    // strcpy(opIpProgPacket.netMask[1], "");
    // strcpy(opIpProgPacket.netMask[2], "");
    // strcpy(opIpProgPacket.netMask[3], "");

    // strcpy(opIpProgPacket.gateWay[1], "");
    // strcpy(opIpProgPacket.gateWay[0], "");
    // strcpy(opIpProgPacket.gateWay[2], "");
    // strcpy(opIpProgPacket.gateWay[3], "");

    opIpProgPacket.enDHCP = true;
    opIpProgPacket.resetConig = true;
   
    if(results.size() > 0)
    {
        std::cout << "Sending ArtAdress packet" << std::endl;
        Artnet::sendOpIpProg(results[0], opIpProgPacket);
    }
    else
    {
        std::cout << "No nodes to configure" << std::endl;
    }
    #endif

}
