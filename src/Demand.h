#ifndef DEMAND_H
#define DEMAND_H
#include <vector>
#include <string>
#include <map>
#include "Supply.h"

class Demand {
public:
    Demand(Supply *supply);
    ~Demand();
    bool Init(const char *file_name);
    bool GetDemand(std::string &demand_node, int &v);
    bool GetPenalty(std::string &demand_node, float &v);
    bool GetTargetSupply(std::string &demand_node, std::vector<std::string> &supply_node);
    std::vector<std::string> GetAllDemandNode();
    float GetV(std::string &node) { return 1.0;}
    
private:
    void _SetSupplySatisfyDemand();
    
private:
    Supply *supply_;
    std::map<std::string, int> demand_;
    std::map<std::string, float> penalty_;
    std::map<std::string, std::vector<std::string> > target_supply_;
};

#endif
