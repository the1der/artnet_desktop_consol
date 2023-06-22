#include <cstdio>
#include <iostream>
#include "artnet_scanner.h"

int main()
{

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



    std::vector <node_t> results;
    results = Artnet::runScan();
    std::cout << std::endl << std::endl << "Results: " << std::endl;
    Artnet::printNodes(results);
    // Artnet::printNodes(Artnet::result_to_str(results));
    if(results.size() > 0)
    {
        std::cout << "Sending ArtAdress packet" << std::endl;
        Artnet::sendArtAdress(results[0], artAddressPacket);
    }
    else
    {
        std::cout << "No nodes to configure" << std::endl;
    }
}
