#include "Backtester.h"
#include <algorithm>
#include <numeric>
#include <cmath>

Backtester::Backtester(std::shared_ptr<EnsembleModel> model) : model_(model) {}

BacktestResult Backtester::run(const std::vector<double>& prices,
                             const std::vector<double>& timestamps,
                             double initial_capital) {
    BacktestResult result;
    std::vector<Trade> trades;
    std::vector<double> equity_curve = {initial_capital};
    double current_capital = initial_capital;
    double position = 0.0;
    
    for (size_t i = 0; i < prices.size(); ++i) {
        // Prepare features for prediction (simplified for now)
        std::vector<double> features = {prices[i]};
        
        // Get prediction from ensemble model
        auto prediction = model_->predict(features);
        
        // Execute trade if confidence is high enough
        if (prediction.confidence > 0.7) {
            Trade trade{
                prediction.action,
                prices[i],
                timestamps[i],
                prediction.confidence
            };
            
            // Simulate trade execution
            if (prediction.action == "BUY" && position <= 0) {
                position = current_capital / prices[i];
                current_capital = 0;
            } else if (prediction.action == "SELL" && position >= 0) {
                current_capital = position * prices[i];
                position = 0;
            }
            
            trades.push_back(trade);
        }
        
        // Update equity curve
        double current_equity = current_capital + (position * prices[i]);
        equity_curve.push_back(current_equity);
    }
    
    // Calculate performance metrics
    result.trades = trades;
    result.total_trades = trades.size();
    result.total_return = (equity_curve.back() - initial_capital) / initial_capital;
    result.sharpe_ratio = calculateSharpeRatio(equity_curve);
    result.max_drawdown = calculateMaxDrawdown(equity_curve);
    result.win_rate = calculateWinRate(trades);
    
    return result;
}

double Backtester::calculateSharpeRatio(const std::vector<double>& returns) {
    if (returns.size() < 2) return 0.0;
    
    // Calculate daily returns
    std::vector<double> daily_returns;
    for (size_t i = 1; i < returns.size(); ++i) {
        daily_returns.push_back((returns[i] - returns[i-1]) / returns[i-1]);
    }
    
    // Calculate mean and standard deviation
    double mean = std::accumulate(daily_returns.begin(), daily_returns.end(), 0.0) / daily_returns.size();
    double variance = 0.0;
    for (double ret : daily_returns) {
        variance += std::pow(ret - mean, 2);
    }
    variance /= daily_returns.size();
    
    return mean / std::sqrt(variance);
}

double Backtester::calculateMaxDrawdown(const std::vector<double>& equity_curve) {
    double max_drawdown = 0.0;
    double peak = equity_curve[0];
    
    for (double value : equity_curve) {
        if (value > peak) {
            peak = value;
        }
        double drawdown = (peak - value) / peak;
        max_drawdown = std::max(max_drawdown, drawdown);
    }
    
    return max_drawdown;
}

double Backtester::calculateWinRate(const std::vector<Trade>& trades) {
    if (trades.empty()) return 0.0;
    
    int winning_trades = 0;
    for (size_t i = 1; i < trades.size(); ++i) {
        if (trades[i].price > trades[i-1].price) {
            winning_trades++;
        }
    }
    
    return static_cast<double>(winning_trades) / trades.size();
} 