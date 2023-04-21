#include <cstdio>
#include <iostream>
#include "artnet_scanner.h"

int main()
{
    std::vector <node_t> results;
    results = Artnet::runScan();
    std::cout << std::endl << std::endl << "Results: " << std::endl;
    // Artnet::printNodes(results);
    Artnet::printNodes(Artnet::result_to_str(results));
}
