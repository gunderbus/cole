#ifndef NODES_H
#define NODES_H

#include <string>
#include <vector>

class node {

// public vars
public:

    // Constructor
    node(std::string conditional, int prev, std::vector<int> connectors)
        : conditional(conditional), prev(prev), connectors(connectors) {}

    int getPrev() const {
        return prev;
    }
    
    std::vector<int> getNext() const {
        return connectors;
    }

private:
    std::string conditional;
    int prev;
    std::vector<int> connectors;

};

#endif
