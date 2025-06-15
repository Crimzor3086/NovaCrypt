#pragma once
#include <string>

class TradeExecutor {
public:
    TradeExecutor();
    void execute(const std::string& signal);
}; 