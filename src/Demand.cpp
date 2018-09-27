#include "Demand.h"
#include <stdlib.h>
#include "common.h"
#include "LineReader.h"

using std::map;
using std::vector;
using std::string;

Demand::Demand(Supply *supply):supply_(supply) {
}

Demand::~Demand() {
}

bool Demand::Init(const char *file_name) {
    LineReader reader(file_name);
    vector<string> items;
    vector<string> supply_nodes;
    while (reader.NextSplitItem(items,'\t')) {
        if (items.size() != 4) {
            continue;
        }

        string demand_node = items[0];
        int target = strtoul(items[1].c_str(), NULL, 10);
        float penalty = strtof(items[2].c_str(), NULL);
        reader.Split(items[3], supply_nodes, ',');

        demand_[demand_node] = target;
        penalty_[demand_node] = penalty;
        target_supply_[demand_node] = supply_nodes;
    }

    _SetSupplySatisfyDemand();
    logger("load %s, num:%u", file_name, demand_.size());
}

bool Demand::GetDemand(string &demand_node, int &v) {
    map<string, int>::iterator iter = demand_.find(demand_node);
    if (iter != demand_.end()) {
        v = iter->second;
        return true;
    }
    return false;
}

bool Demand::GetPenalty(string &demand_node, float &v) {
    map<std::string, float>::iterator iter = penalty_.find(demand_node);
    if (iter != penalty_.end()) {
        v = iter->second;
        return true;
    }

    return false;
}

bool Demand::GetTargetSupply(string &demand_node, vector<string> &supply_node) {
    map<std::string, std::vector<std::string> >::iterator iter =
        target_supply_.find(demand_node);
    if (iter != target_supply_.end()) {
        supply_node = iter->second;
        return true;
    }

    return false;
}

vector<string> Demand::GetAllDemandNode() {
    vector<string> demand_nodes;
    map<std::string, std::vector<std::string> >::iterator iter =
        target_supply_.begin();
    while (iter != target_supply_.end()) {
        demand_nodes.push_back(iter->first);
        ++iter;
    }

    return demand_nodes;
}

void Demand::_SetSupplySatisfyDemand() {
    map<std::string, std::vector<std::string> >::iterator iter =
        target_supply_.begin();
    while (iter != target_supply_.end()) {
        vector<string> &supply_nodes = iter->second;
        for (size_t i=0; i<supply_nodes.size(); ++i) {
            supply_->AddDemandNode(supply_nodes[i], iter->first);
            logger("supply:%s, demand:%s", supply_nodes[i].c_str(), iter->first.c_str());
        }
        ++iter;
    }
}

