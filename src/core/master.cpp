#include "master.hpp"
#include "worker.hpp"
#include <cassert>

namespace core {

master::master(const std::string& conf) {
    assert(load_config(conf) && "Unable to load configuration\n");
    
}

master::~master() {

}

int master::start() {

}

}