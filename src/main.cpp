
#include "master.hpp"

int main(int, const char** argv) {

    const char* conf = argv[1];
    if (!conf) conf = "config/default.conf";

    core::master master(conf);
    
    master.start();
    return 0;
}
