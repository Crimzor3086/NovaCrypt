#pragma once
#include <string>
#include <memory>
#include "ai/EnsembleModel.h"

class AIEngine {
public:
    struct Decision {
        std::string action;
        double confidence;
    };

    AIEngine();
    Decision decide(const std::string& price);
    void updateModelWeights(double rf_performance, double lstm_performance);

private:
    std::shared_ptr<EnsembleModel> model_;
    double last_trade_time_;
    double cooldown_period_;  // Minimum time between trades in seconds
}; 