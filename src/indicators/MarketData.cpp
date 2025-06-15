#include "MarketData.h"
#include <numeric>
#include <cmath>
#include <algorithm>

namespace novacrypt {

// MovingAverage implementation
MovingAverage::MovingAverage(int period) : period_(period) {}

void MovingAverage::update(const OHLCV& data) {
    values_.push_back(data.close);
    if (values_.size() > period_) {
        values_.erase(values_.begin());
    }
}

double MovingAverage::getValue() const {
    if (values_.empty()) return 0.0;
    return std::accumulate(values_.begin(), values_.end(), 0.0) / values_.size();
}

std::string MovingAverage::getName() const {
    return "MA";
}

// SMA implementation
SMA::SMA(int period) : MovingAverage(period) {}

void SMA::update(const OHLCV& data) {
    MovingAverage::update(data);
}

std::string SMA::getName() const {
    return "SMA";
}

// EMA implementation
EMA::EMA(int period) : MovingAverage(period), alpha_(2.0 / (period + 1)) {}

void EMA::update(const OHLCV& data) {
    if (values_.empty()) {
        values_.push_back(data.close);
    } else {
        double ema = alpha_ * data.close + (1 - alpha_) * values_.back();
        values_.push_back(ema);
        if (values_.size() > period_) {
            values_.erase(values_.begin());
        }
    }
}

std::string EMA::getName() const {
    return "EMA";
}

// RSI implementation
RSI::RSI(int period) : period_(period), avgGain_(0.0), avgLoss_(0.0) {}

void RSI::update(const OHLCV& data) {
    if (values_.empty()) {
        values_.push_back(data.close);
        return;
    }

    double change = data.close - values_.back();
    values_.push_back(data.close);

    if (change >= 0) {
        gains_.push_back(change);
        losses_.push_back(0.0);
    } else {
        gains_.push_back(0.0);
        losses_.push_back(-change);
    }

    if (gains_.size() > period_) {
        gains_.erase(gains_.begin());
        losses_.erase(losses_.begin());
    }

    // Calculate average gain and loss
    avgGain_ = std::accumulate(gains_.begin(), gains_.end(), 0.0) / gains_.size();
    avgLoss_ = std::accumulate(losses_.begin(), losses_.end(), 0.0) / losses_.size();
}

double RSI::getValue() const {
    if (avgLoss_ == 0.0) return 100.0;
    double rs = avgGain_ / avgLoss_;
    return 100.0 - (100.0 / (1.0 + rs));
}

std::string RSI::getName() const {
    return "RSI";
}

// MACD implementation
MACD::MACD(int fastPeriod, int slowPeriod, int signalPeriod)
    : fastEMA_(fastPeriod), slowEMA_(slowPeriod), signalEMA_(signalPeriod),
      macdLine_(0.0), signalLine_(0.0) {}

void MACD::update(const OHLCV& data) {
    fastEMA_.update(data);
    slowEMA_.update(data);
    
    macdLine_ = fastEMA_.getValue() - slowEMA_.getValue();
    
    // Create a dummy OHLCV for signal line calculation
    OHLCV signalData = data;
    signalData.close = macdLine_;
    signalEMA_.update(signalData);
    
    signalLine_ = signalEMA_.getValue();
}

double MACD::getValue() const {
    return macdLine_;
}

double MACD::getSignal() const {
    return signalLine_;
}

double MACD::getHistogram() const {
    return macdLine_ - signalLine_;
}

std::string MACD::getName() const {
    return "MACD";
}

// Bollinger Bands implementation
BollingerBands::BollingerBands(int period, double stdDev)
    : period_(period), stdDev_(stdDev), sma_(period) {}

void BollingerBands::update(const OHLCV& data) {
    values_.push_back(data.close);
    if (values_.size() > period_) {
        values_.erase(values_.begin());
    }
    sma_.update(data);
}

double BollingerBands::getValue() const {
    return getMiddleBand();
}

double BollingerBands::getUpperBand() const {
    double middle = getMiddleBand();
    double std = calculateStandardDeviation();
    return middle + (stdDev_ * std);
}

double BollingerBands::getLowerBand() const {
    double middle = getMiddleBand();
    double std = calculateStandardDeviation();
    return middle - (stdDev_ * std);
}

double BollingerBands::getMiddleBand() const {
    return sma_.getValue();
}

double BollingerBands::calculateStandardDeviation() const {
    if (values_.empty()) return 0.0;
    
    double mean = std::accumulate(values_.begin(), values_.end(), 0.0) / values_.size();
    double variance = 0.0;
    
    for (double value : values_) {
        variance += std::pow(value - mean, 2);
    }
    variance /= values_.size();
    
    return std::sqrt(variance);
}

std::string BollingerBands::getName() const {
    return "BollingerBands";
}

// ATR implementation
ATR::ATR(int period) : period_(period), currentATR_(0.0) {}

void ATR::update(const OHLCV& data) {
    if (values_.empty()) {
        values_.push_back(data.close);
        return;
    }

    double high_low = data.high - data.low;
    double high_close = std::abs(data.high - values_.back());
    double low_close = std::abs(data.low - values_.back());
    
    double trueRange = std::max({high_low, high_close, low_close});
    trueRanges_.push_back(trueRange);
    
    if (trueRanges_.size() > period_) {
        trueRanges_.erase(trueRanges_.begin());
    }
    
    currentATR_ = std::accumulate(trueRanges_.begin(), trueRanges_.end(), 0.0) / trueRanges_.size();
    values_.push_back(data.close);
}

double ATR::getValue() const {
    return currentATR_;
}

std::string ATR::getName() const {
    return "ATR";
}

} // namespace novacrypt 