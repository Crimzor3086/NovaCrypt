#include "IndicatorManager.h"
#include <algorithm>
#include <numeric>

namespace novacrypt {

IndicatorManager::IndicatorManager() {
    initializeIndicators();
}

void IndicatorManager::initializeIndicators() {
    // Initialize with default parameters
    rsi_ = std::make_unique<RSI>(14);
    macd_ = std::make_unique<MACD>(12, 26, 9);
    bb_ = std::make_unique<BollingerBands>(20, 2.0);
    atr_ = std::make_unique<ATR>(14);
    
    // Initialize common moving averages
    smas_[20] = std::make_unique<SMA>(20);
    smas_[50] = std::make_unique<SMA>(50);
    smas_[200] = std::make_unique<SMA>(200);
    
    emas_[12] = std::make_unique<EMA>(12);
    emas_[26] = std::make_unique<EMA>(26);
}

void IndicatorManager::update(const OHLCV& data) {
    rsi_->update(data);
    macd_->update(data);
    bb_->update(data);
    atr_->update(data);
    
    for (auto& sma : smas_) {
        sma.second->update(data);
    }
    
    for (auto& ema : emas_) {
        ema.second->update(data);
    }
}

double IndicatorManager::getIndicatorValue(const std::string& name) const {
    if (name == "RSI") return getRSI();
    if (name == "MACD") return getMACD();
    if (name == "MACD_SIGNAL") return getMACDSignal();
    if (name == "MACD_HIST") return getMACDHistogram();
    if (name == "BB_UPPER") return getBBUpper();
    if (name == "BB_MIDDLE") return getBBMiddle();
    if (name == "BB_LOWER") return getBBLower();
    if (name == "ATR") return getATR();
    return 0.0;
}

std::vector<double> IndicatorManager::getFeatureVector() const {
    std::vector<double> features;
    
    // Add all indicator values
    features.push_back(getRSI());
    features.push_back(getMACD());
    features.push_back(getMACDSignal());
    features.push_back(getMACDHistogram());
    features.push_back(getBBUpper());
    features.push_back(getBBMiddle());
    features.push_back(getBBLower());
    features.push_back(getATR());
    
    // Add moving averages
    for (const auto& sma : smas_) {
        features.push_back(sma.second->getValue());
    }
    
    for (const auto& ema : emas_) {
        features.push_back(ema.second->getValue());
    }
    
    // Add order book metrics
    features.push_back(getBidAskSpread());
    features.push_back(getOrderImbalance());
    features.push_back(getSlippageEstimate());
    
    return features;
}

double IndicatorManager::getRSI() const {
    return rsi_->getValue();
}

double IndicatorManager::getMACD() const {
    return macd_->getValue();
}

double IndicatorManager::getMACDSignal() const {
    return macd_->getSignal();
}

double IndicatorManager::getMACDHistogram() const {
    return macd_->getHistogram();
}

double IndicatorManager::getBBUpper() const {
    return bb_->getUpperBand();
}

double IndicatorManager::getBBMiddle() const {
    return bb_->getMiddleBand();
}

double IndicatorManager::getBBLower() const {
    return bb_->getLowerBand();
}

double IndicatorManager::getATR() const {
    return atr_->getValue();
}

double IndicatorManager::getSMA(int period) const {
    auto it = smas_.find(period);
    if (it != smas_.end()) {
        return it->second->getValue();
    }
    return 0.0;
}

double IndicatorManager::getEMA(int period) const {
    auto it = emas_.find(period);
    if (it != emas_.end()) {
        return it->second->getValue();
    }
    return 0.0;
}

void IndicatorManager::updateOrderBook(const OrderBook& orderBook) {
    currentOrderBook_ = orderBook;
}

double IndicatorManager::getBidAskSpread() const {
    if (currentOrderBook_.bids.empty() || currentOrderBook_.asks.empty()) {
        return 0.0;
    }
    
    double bestBid = currentOrderBook_.bids.front().price;
    double bestAsk = currentOrderBook_.asks.front().price;
    
    return bestAsk - bestBid;
}

double IndicatorManager::getOrderImbalance() const {
    if (currentOrderBook_.bids.empty() || currentOrderBook_.asks.empty()) {
        return 0.0;
    }
    
    double bidVolume = std::accumulate(currentOrderBook_.bids.begin(), currentOrderBook_.bids.end(),
                                     0.0, [](double sum, const OrderBookLevel& level) {
                                         return sum + level.quantity;
                                     });
    
    double askVolume = std::accumulate(currentOrderBook_.asks.begin(), currentOrderBook_.asks.end(),
                                     0.0, [](double sum, const OrderBookLevel& level) {
                                         return sum + level.quantity;
                                     });
    
    return (bidVolume - askVolume) / (bidVolume + askVolume);
}

double IndicatorManager::getSlippageEstimate() const {
    if (currentOrderBook_.bids.empty() || currentOrderBook_.asks.empty()) {
        return 0.0;
    }
    
    double midPrice = (currentOrderBook_.bids.front().price + currentOrderBook_.asks.front().price) / 2.0;
    double spread = getBidAskSpread();
    
    // Estimate slippage based on order book depth and spread
    return spread * (1.0 + std::abs(getOrderImbalance()));
}

} // namespace novacrypt 