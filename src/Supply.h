#ifndef __SUPPLY_H
#define __SUPPLY_H
#include <map>
#include <string>
#include <vector>

class Supply {
public:
    bool Init(const char *file_name);
    int GetSupply(std::string &node);
    bool GetSatisfyDemand(std::string &node, std::vector<std::string>&demands);
    std::vector<std::string> GetAllSupply();
    void AddDemandNode(const std::string &supply_node, const std::string &demand_node);
private:
    std::map<std::string, int> node2supply_;
    std::map<std::string, std::vector<std::string> > satisfy_demand_;
};
#endif
