#pragma once
#include <vector>
#include <string>
#include <chrono>

namespace novacrypt {

struct OHLCV {
    double open;
    double high;
    double low;
    double close;
    double volume;
    std::chrono::system_clock::time_point timestamp;
};

struct OrderBookLevel {
    double price;
    double quantity;
};

struct OrderBook {
    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;
    std::chrono::system_clock::time_point timestamp;
};

class Indicator {
public:
    virtual ~Indicator() = default;
    virtual void update(const OHLCV& data) = 0;
    virtual double getValue() const = 0;
    virtual std::string getName() const = 0;
};

// Moving Averages
class MovingAverage : public Indicator {
public:
    MovingAverage(int period);
    void update(const OHLCV& data) override;
    double getValue() const override;
    std::string getName() const override;

protected:
    int period_;
    std::vector<double> values_;
};

class SMA : public MovingAverage {
public:
    SMA(int period);
    void update(const OHLCV& data) override;
    std::string getName() const override;
};

class EMA : public MovingAverage {
public:
    EMA(int period);
    void update(const OHLCV& data) override;
    std::string getName() const override;

private:
    double alpha_;  // Smoothing factor
};

// RSI
class RSI : public Indicator {
public:
    RSI(int period);
    void update(const OHLCV& data) override;
    double getValue() const override;
    std::string getName() const override;

private:
    int period_;
    std::vector<double> gains_;
    std::vector<double> losses_;
    double avgGain_;
    double avgLoss_;
};

// MACD
class MACD : public Indicator {
public:
    MACD(int fastPeriod, int slowPeriod, int signalPeriod);
    void update(const OHLCV& data) override;
    double getValue() const override;
    std::string getName() const override;
    double getSignal() const;
    double getHistogram() const;

private:
    EMA fastEMA_;
    EMA slowEMA_;
    EMA signalEMA_;
    double macdLine_;
    double signalLine_;
};

// Bollinger Bands
class BollingerBands : public Indicator {
public:
    BollingerBands(int period, double stdDev);
    void update(const OHLCV& data) override;
    double getValue() const override;
    std::string getName() const override;
    double getUpperBand() const;
    double getLowerBand() const;
    double getMiddleBand() const;

private:
    int period_;
    double stdDev_;
    SMA sma_;
    std::vector<double> values_;
};

// ATR
class ATR : public Indicator {
public:
    ATR(int period);
    void update(const OHLCV& data) override;
    double getValue() const override;
    std::string getName() const override;

private:
    int period_;
    std::vector<double> trueRanges_;
    double currentATR_;
};

} // namespace novacrypt 