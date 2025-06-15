#pragma once
#include <string>
#include <vector>
#include <memory>

class EnsembleModel {
public:
    struct Prediction {
        std::string action;  // BUY, SELL, HOLD
        double confidence;   // Confidence score (0.0 to 1.0)
        double rf_weight;    // Random Forest weight
        double lstm_weight;  // LSTM weight
    };

    EnsembleModel();
    Prediction predict(const std::vector<double>& features);
    void updateWeights(double rf_performance, double lstm_performance);

private:
    // Model weights (can be adjusted based on performance)
    double rf_weight_;
    double lstm_weight_;
    
    // Simulated model predictions (to be replaced with actual implementations)
    std::string predictRF(const std::vector<double>& features);
    std::string predictLSTM(const std::vector<double>& features);
    
    // Helper function to calculate confidence score
    double calculateConfidence(const std::string& rf_pred, const std::string& lstm_pred);
}; 