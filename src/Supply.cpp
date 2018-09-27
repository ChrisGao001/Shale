#include "Supply.h"
#include <stdlib.h>
#include "LineReader.h"
#include "common.h"

using std::vector;
using std::map;
using std::string;

bool Supply::Init(const char *file_name) {
    if (file_name == NULL) {
        return false;
    }
    
    LineReader reader(file_name);
    vector<string> items;
    while (reader.NextSplitItem(items, '\t')) {
        if (items.size() != 2) {
            continue;
        }

        node2supply_[items[0]] = strtoul(items[1].c_str(), NULL, 10);
    }
    logger("load %s, num:%u", file_name, node2supply_.size());
    return true;
}

int Supply::GetSupply(std::string &node) {
    int supply = -1;
    map<string, int>::iterator iter = node2supply_.find(node);
    if (iter != node2supply_.end()) {
        supply = iter->second;
    }

    return supply;
}

bool Supply::GetSatisfyDemand(std::string &node, std::vector<string>&demands) {
    map<string, vector<string> >::iterator iter = satisfy_demand_.find(node);
    if (iter != satisfy_demand_.end()) {
        demands = iter->second;
        return true;
    }

    return false;
}
 
std::vector<std::string> Supply::GetAllSupply() {
    vector<string> result;
    map<string, int>::iterator iter = node2supply_.begin();
    while (iter != node2supply_.end()) {
        result.push_back(iter->first);
        ++iter;
    }
    return result;
}

void Supply::AddDemandNode(const std::string &supply_node, const std::string &demand_node) {
    std::map<std::string, std::vector<std::string> >::iterator iter = satisfy_demand_.find(supply_node);
    if (iter == satisfy_demand_.end()) {
        vector<string> item;
        item.push_back(demand_node);
        satisfy_demand_[supply_node] = item;
    } else {
        vector<string> &item = satisfy_demand_[supply_node];
        item.push_back(demand_node);
    }
}

