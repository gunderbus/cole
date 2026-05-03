#include "nodes.h"

node::node(std::string conditional, int prev, std::vector<int> connectors)
    : conditional(conditional), prev(prev), connectors(connectors) {}

std::string node::getConditional() const {
    return conditional;
}

int node::getPrev() const {
    return prev;
}

std::vector<int> node::getNext() const {
    return connectors;
}