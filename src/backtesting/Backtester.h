#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../ai/EnsembleModel.h"

struct Trade {
    std::string action;
    double price;
    double timestamp;
    double confidence;
};

struct BacktestResult {
    double total_return;
    double sharpe_ratio;
    double max_drawdown;
    int total_trades;
    double win_rate;
    std::vector<Trade> trades;
};

class Backtester {
public:
    Backtester(std::shared_ptr<EnsembleModel> model);
    BacktestResult run(const std::vector<double>& prices,
                      const std::vector<double>& timestamps,
                      double initial_capital = 10000.0);

private:
    std::shared_ptr<EnsembleModel> model_;
    
    // Helper functions
    double calculateSharpeRatio(const std::vector<double>& returns);
    double calculateMaxDrawdown(const std::vector<double>& equity_curve);
    double calculateWinRate(const std::vector<Trade>& trades);
}; 