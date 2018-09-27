#include "Shale.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <sstream>

using std::sort;
using std::string;
using std::map;
using std::vector;

float max(float a, float b) { return (a>b ? a : b);}
float min(float a, float b) { return (a<b ? a : b);}
bool sigmaCmp1(const sigma_coef_t &left, const sigma_coef_t &right) { return (left.r1 < right.r1);}
bool sigmaCmp2(const sigma_coef_t &left, const sigma_coef_t &right) { return (left.r2 < right.r2);}

void Shale::Init(Supply *supply, Demand *demand) {
    supply_ = supply;
    demand_ = demand;
    vector<string> all_demand_nodes = demand_->GetAllDemandNode();
    vector<string> supply_nodes;
    for (size_t i=0; i<all_demand_nodes.size(); ++i) {
        string &demand_node = all_demand_nodes[i];
        float sum = 0.0;
        if (!demand_->GetTargetSupply(demand_node, supply_nodes)) {
            continue;
        }
        for (size_t j=0; j<supply_nodes.size(); ++j) {
            string supply_node = supply_nodes[j];
            sum += supply_->GetSupply(supply_node);
        }
        int v = 0;
        if (demand_->GetDemand(demand_node, v)) {
            theta_ij_[demand_node] = v / sum;
        }
    }
}
void Shale::StageOne(int iteration) {
    for (int iter=0; iter<iteration; ++iter) {
        vector<string> all_supply_nodes = supply_->GetAllSupply();
        //logger("iter:%d", iter);
        for (size_t i=0; i<all_supply_nodes.size(); ++i) {
            UpdateBeta(all_supply_nodes[i]);
        }
        vector<string> all_demand_nodes = demand_->GetAllDemandNode();
        for (size_t j=0; j<all_demand_nodes.size(); ++j) {
            UpdateAlpha(all_demand_nodes[j]);
        }
    }
    Output();
}

void Shale::StageTwo() {
    vector<string> all_supply_nodes = supply_->GetAllSupply();
    for (size_t i=0; i<all_supply_nodes.size(); ++i) {
        string &supply_node = all_supply_nodes[i];
        s_i_[supply_node] = supply_->GetSupply(supply_node);
        logger("supply_node:%s, supply=%d", supply_node.c_str(), supply_->GetSupply(supply_node));
        UpdateBeta(supply_node);
    }

    vector<string> all_demand_nodes = demand_->GetAllDemandNode();
    // for debug
    all_demand_nodes.clear();
    all_demand_nodes.push_back("a");
    all_demand_nodes.push_back("c");
    all_demand_nodes.push_back("b");
    vector<string> supply_nodes;
    for (size_t j=0; j<all_demand_nodes.size(); ++j) {
        string &demand_node = all_demand_nodes[j];
        FindSigma(demand_node);
        if (!demand_->GetTargetSupply(demand_node,supply_nodes)) {
            continue;
        }
        for (size_t i=0; i<supply_nodes.size(); ++i) {
            string &supply_node = supply_nodes[i];
            float g = max(0, theta_ij_[demand_node] * (1.0 + (sigma_j_[demand_node] - 
                beta_i_[supply_node])/demand_->GetV(demand_node)));
            s_i_[supply_node] -= min(s_i_[supply_node], supply_->GetSupply(supply_node) * g);
        }
    }

}

void Shale::Output() {
    map<string, float>::iterator iter = alpha_j_.begin();
    std::ostringstream oss;
    oss << "alpha:\n";
    while (iter != alpha_j_.end()) {
        oss << " " <<iter->first << ":" << iter->second;
        ++iter;
    }
    oss << "\n beta:\n";
    iter = beta_i_.begin();
    while (iter != beta_i_.end()) {
        oss << " " << iter->first << ":" << iter->second;
        ++iter;
    }
    oss << "\n sigma:\n";
    iter = sigma_j_.begin();
    while (iter != sigma_j_.end()) {
        oss << " " << iter->first << ":" << iter->second;
        ++iter;
    }
    logger("%s", oss.str().c_str());
}

void Shale::UpdateBeta(std::string &supply_node) {
    vector<coef_t> coefs;
    vector<string> demand_nodes;
    if (! supply_->GetSatisfyDemand(supply_node, demand_nodes)) {
        return;
    }
    for (size_t j=0; j<demand_nodes.size(); ++j) {
        string &demand_node = demand_nodes[j];
        float a = theta_ij_[demand_node] * (1.0 + alpha_j_[demand_node]) / demand_->GetV(demand_node);
        float b = theta_ij_[demand_node] / demand_->GetV(demand_node);
        coefs.push_back(coef_t(a, b, 0, a/b));
        //logger("UpdateBeta supply_node:%s, demand_node:%s, a:%f, b:%f, r:%f", supply_node.c_str(),demand_node.c_str(), a, b, a/b);
    }

    vector<float> solution = Solution::Solve(coefs, 1.0);
    if (solution.empty() || solution[0] < 0.0000001) {
        beta_i_[supply_node] = 0;
    } else {
        beta_i_[supply_node] = solution[0];
    }

}

void Shale::UpdateAlpha(std::string &demand_node) {
    vector<coef_t> coefs;
    vector<string> supply_nodes;
    int v;
    float penal;
    
    if (! demand_->GetTargetSupply(demand_node, supply_nodes)) {
        return;
    }

    if (!demand_->GetDemand(demand_node, v) && !demand_->GetPenalty(demand_node, penal)) {
        return;
    }
    
    for (size_t i=0; i<supply_nodes.size(); ++i) {
        string &supply_node = supply_nodes[i];
        int s = supply_->GetSupply(supply_node);
        float a = -1.0 * s * theta_ij_[demand_node] * (1.0 - beta_i_[supply_node] / demand_->GetV(demand_node));
        float b = -1.0 * s * theta_ij_[demand_node] / demand_->GetV(demand_node);
        coefs.push_back(coef_t(a, b, 0, a/b));
    }

    vector<float> solution = Solution::Solve(coefs, -1.0 * v);
    if (solution.size() == 0 && solution[0] > penal) {
        alpha_j_[demand_node] = penal;
    } else {
        alpha_j_[demand_node] = -1.0 * solution[0];
    }

}

void Shale::FindSigma(std::string &demand_node) {
    vector<float> result = UpdateSigma(demand_node);
    if (result.empty()) {
        sigma_j_[demand_node] = INFINITY;
    } else {
        sigma_j_[demand_node] = -1.0 * result[0];
    }
}

std::vector<float> Shale::UpdateSigma(std::string &demand_node) {
    vector<float> solution;
    vector<sigma_coef_t> coefs;
    vector<string> supply_nodes;
    int v = 0;
    if (!demand_->GetTargetSupply(demand_node, supply_nodes)) {
        return solution;
    }

    if  (!demand_->GetDemand(demand_node, v)) {
        return solution;
    }

    for (size_t i=0; i<supply_nodes.size(); ++i) {
        string &supply_node = supply_nodes[i];
        int s = supply_->GetSupply(supply_node);
        float a = s * theta_ij_[demand_node] * (1 - beta_i_[supply_node] / demand_->GetV(demand_node));
        float b = s * theta_ij_[demand_node] / demand_->GetV(demand_node);
        coefs.push_back(sigma_coef_t(a, b, s_i_[supply_node], (s_i_[supply_node] - a)/b, a/b));

        //logger("sigma supply:%s, a:%f,b:%f,c:%f,r1:%f,r2:%f", supply_node.c_str(),a, b, s_i_[supply_node], (s_i_[supply_node] - a)/b, a/b);
    }
    sort(coefs.begin(), coefs.end(), sigmaCmp2);
    for (size_t i=0; i<coefs.size(); ++i) {
        vector<sigma_coef_t> tmp(coefs.begin() + i, coefs.end());
        sort(tmp.begin(), tmp.end(), sigmaCmp1);
        float sum_remained = 0;
        float sum_confs = 0.0;
        float sum_coef = 0.0;
        for (size_t j=0; j<tmp.size(); ++j) {
            sum_confs += tmp[j].a;
            sum_coef += tmp[j].b;
            sum_remained += tmp[j].c;
        }
        float res = (sum_confs - v) / sum_coef;
        if (sum_remained < v) {
            continue;
        }

        if (i == 0) {
            if (res <= tmp[0].r1 && res <= coefs[i].r2) {
                solution.push_back(res);
                //logger("solution:%f",res);
            }
        } else {
            if (res <= tmp[0].r1 && res <= coefs[i].r2 && res >= coefs[i-1].r2) {
                solution.push_back(res);
                //logger("solution:%f",res);
            }
        }
        sum_remained = 0.0;
        for (size_t k=1; k<tmp.size(); ++k) {
            sum_confs -= tmp[k-1].a;
            sum_coef -= tmp[k-1].b;
            sum_remained += tmp[k-1].c;
            res = (sum_confs + sum_remained - v) / sum_coef;
            if (i == 0) {
                if (res <= tmp[k].r1 && res >= tmp[k-1].r1 && res <= coefs[i].r2) {
                    solution.push_back(res);
                    //logger("solution:%f",res);
                }
            } else {
                 if (res <= tmp[k].r1 && res >= tmp[k-1].r1 && 
                    res <= coefs[i].r2 && res >= coefs[i-1].r2) {
                    solution.push_back(res);
                    //logger("solution:%f",res);
                 }
            }
        }
    }

    return solution;
}



void Online::Init(Supply *supply, Demand *demand, std::map<std::string, float> *alpha, 
    std::map<std::string, float> *sigma) {
    supply_ = supply;
    demand_ = demand;
    alpha_j_ = alpha;
    sigma_j_ = sigma;
    vector<string> all_supply_nodes = supply_->GetAllSupply();
    for (size_t i=0; i<all_supply_nodes.size(); ++i) {
        string &supply_node = all_supply_nodes[i];
        remaind_i_[supply_node ] = supply_->GetSupply(supply_node);
    }

    vector<string> all_demand_nodes = demand_->GetAllDemandNode();
    vector<string> supply_nodes;
    for (size_t i=0; i<all_demand_nodes.size(); ++i) {
        string &demand_node = all_demand_nodes[i];
        float sum = 0.0;
        if (!demand_->GetTargetSupply(demand_node, supply_nodes)) {
            continue;
        }
        for (size_t j=0; j<supply_nodes.size(); ++j) {
            string supply_node = supply_nodes[j];
            sum += supply_->GetSupply(supply_node);
        }
        int v = 0;
        if (demand_->GetDemand(demand_node, v)) {
            theta_ij_[demand_node] = 1.0 * v / sum;
            //logger("theta_ij demand_node:%s, theta:%f", demand_node.c_str(), 1.0* v/sum);
        }
        allocation_j_[demand_node] = 0;
    }
    srand(time(NULL));

}


void Online::Allocate(std::string &supply_node) {
    float s = 1.0;
    map<string, float> x_ij;

    if (beta_i_.find(supply_node) == beta_i_.end()) {
        UpdateBeta(supply_node);
    }

    vector<string> demand_nodes;
    if (!supply_->GetSatisfyDemand(supply_node, demand_nodes)) {
        return;
    }

    for (size_t j=0; j<demand_nodes.size(); ++j) {
        string &demand_node = demand_nodes[j];
        float theta = theta_ij_[demand_node] * (1.0
        + ((*sigma_j_)[demand_node] - beta_i_[supply_node]) / demand_->GetV(demand_node));
        float g = max(0, theta);
        x_ij[demand_node] = min(s, g);
        s -= x_ij[demand_node];
        //logger("demand_node:%s,supply_node:%s,g:%f, x_ij:%f,s:%f", demand_node.c_str(), supply_node.c_str(),g, x_ij[demand_node],s);
    }

    float sum = 0.0;
    map<string, float>::iterator iter = x_ij.begin();
    while (iter != x_ij.end()) {
        sum += iter->second;
        //logger("demand:%s, x_ij:%f", iter->first.c_str(), iter->second);
        ++iter;
    }

    if (sum < 1.0) {
        logger("there is %f chance that no conract is selected", 1-sum);
    }
    sum = 0.0;
    float r = 1.0 * random()/ RAND_MAX;
    for (iter = x_ij.begin(); iter != x_ij.end(); ++iter) {
        sum += iter->second;
        if (sum > r) {
            string demand_node = iter->first;
            allocation_j_[demand_node] += 1;
            remaind_i_[supply_node] -= 1;
            //logger("allocate supply %s to demand %s", supply_node.c_str(), demand_node.c_str());
            break;
        }
    }
}

void Online::UpdateBeta(std::string &supply_node) {
    vector<coef_t> coefs;
    vector<string> demand_nodes;
    if (! supply_->GetSatisfyDemand(supply_node, demand_nodes)) {
        return;
    }
    for (size_t j=0; j<demand_nodes.size(); ++j) {
        string &demand_node = demand_nodes[j];
        float a = theta_ij_[demand_node] * (1+ (*alpha_j_)[demand_node]) / demand_->GetV(demand_node);
        float b = theta_ij_[demand_node] / demand_->GetV(demand_node);
        coefs.push_back(coef_t(a, b, 0, a/b));
    }

    vector<float> solution = Solution::Solve(coefs, 1.0);
    if (solution.empty() || solution[0] < 0.0000001) {
        beta_i_[supply_node] = 0;
    } else {
        beta_i_[supply_node] = solution[0];
        //logger("beta_i supply_node:%s, value=%f", supply_node.c_str(), solution[0]);
    }
}

void Online::Output() {
    std::ostringstream oss;
    oss << "allocation:\n";
    map<string, int>::iterator iter = allocation_j_.begin();
    while (iter != allocation_j_.end()) {
        oss << " " << iter->first << ":" << iter->second;
        ++iter;
    }

    oss << "\n remained_i\n";
    iter = remaind_i_.begin();
    while (iter != remaind_i_.end()) {
        oss << " " << iter->first << ":" << iter->second;
        ++iter;
    }
    logger("Online: %s", oss.str().c_str());
}
vector<float> Solution::Solve(vector<coef_t> &coefs, float d) {
    vector<float> ret;
    double sum_a = 0.0;
    double sum_b = 0.0;
    float a = 0.0;
    float b = 0.0;
    float c = 0.0;
    float x = 0.0;

    sort(coefs.begin(), coefs.end());
    for (size_t i=0; i<coefs.size(); ++i) {
        sum_a += coefs[i].a;
        sum_b += coefs[i].b;
    }

    size_t i = 0;
    do {
        sum_a -= a;
        sum_b -= b;
        d -= c;
        x = (sum_a - d) / sum_b;
        if (x < coefs[i].r) {
            ret.push_back(x);
        }
        a = coefs[i].a;
        b = coefs[i].b;
        c = coefs[i].c;
        ++i;
    } while (i < coefs.size());
    return ret;
}

