#pragma once
#include <string>

class MarketData {
public:
    MarketData();
    void fetch();
    std::string getLatestPrice() const;
}; 