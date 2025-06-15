#pragma once
#include "MarketData.h"
#include "AIEngine.h"
#include "Strategy.h"
#include "TradeExecutor.h"
#include "RiskManager.h"

class NovaCryptBot {
public:
    NovaCryptBot();
    void run();
private:
    MarketData marketData;
    AIEngine aiEngine;
    Strategy strategy;
    TradeExecutor executor;
    RiskManager riskManager;
}; 