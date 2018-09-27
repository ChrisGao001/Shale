#ifndef SHALE_H
#define SHALE_H
#include <vector>
#include <string>
#include <vector>
#include "common.h"
#include "Supply.h"
#include "Demand.h"
typedef struct coef_t coef_t;

struct coef_t {
    float a;
    float b;
    float c;
    float r;
    coef_t(float _a=0,float _b=0, float _c=0, float _r=0) { a = _a;b=_b;c=_c;r=_r;}
    bool operator <(const coef_t &other) const {return (r < other.r);}
};

struct sigma_coef_t {
    float a;
    float b;
    float c;
    float r1;
    float r2;
    sigma_coef_t(float _a=0,float _b=0, float _c=0, float _r1=0, float _r2=0) { a = _a;b=_b;c=_c;r1=_r1;r2=_r2;}
};

class Shale {
public:
    void Init(Supply *supply, Demand *demand);
    void StageOne(int iteration);
    void StageTwo();
    void Output();
    void UpdateBeta(std::string &supply_node);
    void UpdateAlpha(std::string &demand_node);
    void FindSigma(std::string &demand_node);
    std::vector<float> UpdateSigma(std::string &demand_node);
    std::map<std::string, float>* GetAlpha() { return &alpha_j_;}
    std::map<std::string, float>* GetSigma() { return &sigma_j_;}

private:
    Supply *supply_;
    Demand *demand_;
    std::map<std::string, float> alpha_j_;
    std::map<std::string, float> sigma_j_;
    std::map<std::string, float> theta_ij_;
    std::map<std::string, float> beta_i_;
    std::map<std::string, float> s_i_;
};

class Online {
public:
    void Init(Supply *supply, Demand *demand, std::map<std::string, float> *alpha, 
        std::map<std::string, float> *sigma);
    void Allocate(std::string &supply_node);
    void UpdateBeta(std::string &supply_node);
    void Output();
private:
    Supply *supply_;
    Demand *demand_;
    std::map<std::string, float> *alpha_j_;
    std::map<std::string, float> *sigma_j_;
    std::map<std::string, float> theta_ij_;
    std::map<std::string, float> beta_i_;
    std::map<std::string, int> allocation_j_;
    std::map<std::string, int> remaind_i_;
};

class Solution {
public:
    static std::vector<float> Solve(std::vector<coef_t> &coefs, float d); 
};

#endif
