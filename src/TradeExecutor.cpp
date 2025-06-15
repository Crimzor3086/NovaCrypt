#include "TradeExecutor.h"
#include <iostream>

TradeExecutor::TradeExecutor() {}

void TradeExecutor::execute(const std::string& signal) {
    std::cout << "Executing trade: " << signal << std::endl;
} 