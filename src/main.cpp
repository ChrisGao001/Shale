#include <iostream>
#include "Shale.h"
#include "common.h"

using namespace std;

int main(int argc, char **argv) {
	Supply supply;
	supply.Init("./supply.txt");

	Demand demand(&supply);
	demand.Init("./demand.txt");
	Shale shale;
	shale.Init(&supply, &demand);
    logger("stage one");
	shale.StageOne(5);
    logger("stage two");
	shale.StageTwo();
    logger("output");
	shale.Output();
	
	Online online;
	online.Init(&supply, &demand, shale.GetAlpha(), shale.GetSigma());
	vector<string> supply_nodes = supply.GetAllSupply();	
    // for debug
    supply_nodes.clear();
    supply_nodes.push_back("1");
    supply_nodes.push_back("0");
    supply_nodes.push_back("3");
    supply_nodes.push_back("2");
	for (size_t i=0; i<supply_nodes.size(); ++i) {
		string &supply_node = supply_nodes[i];
		int inventory = supply.GetSupply(supply_node);
        logger("supply_node:%s, inventory:%d", supply_node.c_str(), inventory);
		while (inventory-- > 0) {
			online.Allocate(supply_node);	
		}	
	}
    online.Output();
	return 0;
}

