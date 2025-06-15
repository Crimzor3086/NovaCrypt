#include "AIEngine.h"
#include <chrono>
#include <ctime>

AIEngine::AIEngine() 
    : model_(std::make_shared<EnsembleModel>()),
      last_trade_time_(0.0),
      cooldown_period_(300.0)  // 5 minutes cooldown
{
}

AIEngine::Decision AIEngine::decide(const std::string& price) {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    double current_time = static_cast<double>(now_time_t);
    
    // Check cooldown period
    if (current_time - last_trade_time_ < cooldown_period_) {
        return Decision{"HOLD", 0.0};  // Force hold during cooldown
    }
    
    // Convert price string to double
    double price_value = std::stod(price);
    
    // Prepare features for prediction
    std::vector<double> features = {price_value};
    
    // Get prediction from ensemble model
    auto prediction = model_->predict(features);
    
    // Update last trade time if we're making a trade
    if (prediction.action != "HOLD") {
        last_trade_time_ = current_time;
    }
    
    return Decision{
        prediction.action,
        prediction.confidence
    };
}

void AIEngine::updateModelWeights(double rf_performance, double lstm_performance) {
    model_->updateWeights(rf_performance, lstm_performance);
} 