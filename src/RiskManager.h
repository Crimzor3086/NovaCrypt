#pragma once
#include <string>

class RiskManager {
public:
    RiskManager();
    bool check(const std::string& signal);
}; 