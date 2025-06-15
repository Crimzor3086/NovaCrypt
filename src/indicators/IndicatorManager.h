#pragma once
#include "MarketData.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace novacrypt {

class IndicatorManager {
public:
    IndicatorManager();
    
    // Update all indicators with new market data
    void update(const OHLCV& data);
    
    // Get indicator values
    double getIndicatorValue(const std::string& name) const;
    
    // Get all current indicator values as a feature vector
    std::vector<double> getFeatureVector() const;
    
    // Get specific indicator values
    double getRSI() const;
    double getMACD() const;
    double getMACDSignal() const;
    double getMACDHistogram() const;
    double getBBUpper() const;
    double getBBMiddle() const;
    double getBBLower() const;
    double getATR() const;
    double getSMA(int period) const;
    double getEMA(int period) const;
    
    // Get order book metrics
    double getBidAskSpread() const;
    double getOrderImbalance() const;
    double getSlippageEstimate() const;
    
    // Update order book data
    void updateOrderBook(const OrderBook& orderBook);

private:
    // Technical indicators
    std::unique_ptr<RSI> rsi_;
    std::unique_ptr<MACD> macd_;
    std::unique_ptr<BollingerBands> bb_;
    std::unique_ptr<ATR> atr_;
    std::unordered_map<int, std::unique_ptr<SMA>> smas_;
    std::unordered_map<int, std::unique_ptr<EMA>> emas_;
    
    // Order book data
    OrderBook currentOrderBook_;
    
    // Initialize indicators with default parameters
    void initializeIndicators();
};

} // namespace novacrypt 