#include "EnsembleModel.h"
#include <algorithm>
#include <cmath>

EnsembleModel::EnsembleModel() : rf_weight_(0.5), lstm_weight_(0.5) {}

EnsembleModel::Prediction EnsembleModel::predict(const std::vector<double>& features) {
    std::string rf_pred = predictRF(features);
    std::string lstm_pred = predictLSTM(features);
    
    // Combine predictions based on weights
    std::string final_action;
    if (rf_pred == lstm_pred) {
        final_action = rf_pred;
    } else {
        // Weighted decision
        final_action = (rf_weight_ > lstm_weight_) ? rf_pred : lstm_pred;
    }
    
    double confidence = calculateConfidence(rf_pred, lstm_pred);
    
    return Prediction{
        final_action,
        confidence,
        rf_weight_,
        lstm_weight_
    };
}

void EnsembleModel::updateWeights(double rf_performance, double lstm_performance) {
    double total = rf_performance + lstm_performance;
    if (total > 0) {
        rf_weight_ = rf_performance / total;
        lstm_weight_ = lstm_performance / total;
    }
}

std::string EnsembleModel::predictRF(const std::vector<double>& features) {
    // TODO: Implement actual Random Forest prediction
    // For now, return a simulated prediction
    return "HOLD";
}

std::string EnsembleModel::predictLSTM(const std::vector<double>& features) {
    // TODO: Implement actual LSTM prediction
    // For now, return a simulated prediction
    return "HOLD";
}

double EnsembleModel::calculateConfidence(const std::string& rf_pred, const std::string& lstm_pred) {
    // Calculate confidence based on agreement between models
    if (rf_pred == lstm_pred) {
        return 0.8;  // High confidence when models agree
    }
    
    // Lower confidence when models disagree
    return 0.4;
} 