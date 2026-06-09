#include "OTAClient.hpp"
#include <thread>
#include <chrono>

int main(int argc, char *argv[]){
    if(argc < 2){
        std::cerr << "Usage: ota-clint <image.img>\n";
        return 1;
    }

    OTAClient client;
    if(!client.connect()) return 1;
    return client.sendImage(argv[1]) ? 0 : 1;
}