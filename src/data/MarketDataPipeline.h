#pragma once
#include "../indicators/MarketData.h"
#include "../sentiment/SentimentAnalyzer.h"
#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "DataQualityMetrics.h"

namespace novacrypt {

struct MarketDataUpdate {
    double price;
    double volume;
    std::chrono::system_clock::time_point timestamp;
    std::string source;
    double confidence;
};

struct OrderBookUpdate {
    struct Level {
        double price;
        double volume;
    };
    std::vector<Level> bids;
    std::vector<Level> asks;
    std::chrono::system_clock::time_point timestamp;
    std::string source;
    double confidence;
};

class MarketDataPipeline {
public:
    MarketDataPipeline();
    ~MarketDataPipeline();
    
    // Start/Stop the pipeline
    void start();
    void stop();
    
    // Data input methods - only accept real data
    void pushMarketData(const MarketDataUpdate& data);
    void pushOrderBook(const OrderBookUpdate& data);
    void pushSentimentData(const std::string& source, double sentiment);
    
    // Get processed data
    MarketDataUpdate getLatestMarketData() const;
    OrderBookUpdate getLatestOrderBook() const;
    double getLatestSentiment(const std::string& source) const;
    
    // Configuration
    void setUpdateInterval(std::chrono::milliseconds interval);
    void setMaxQueueSize(size_t size);
    
    // Data validation
    bool validateMarketData(const MarketDataUpdate& data) const;
    bool validateOrderBook(const OrderBookUpdate& data) const;
    
    // Callbacks for data updates
    using MarketDataCallback = std::function<void(const MarketDataUpdate&)>;
    using OrderBookCallback = std::function<void(const OrderBookUpdate&)>;
    using SentimentCallback = std::function<void(const std::string&, double)>;
    
    void setMarketDataCallback(MarketDataCallback callback);
    void setOrderBookCallback(OrderBookCallback callback);
    void setSentimentCallback(SentimentCallback callback);

    // Data quality methods
    DataQualityMetrics getDataQualityMetrics(const std::string& source) const;
    std::string generateDataQualityReport(const std::string& source) const;
    std::string generateDataQualitySummary() const;

private:
    // Pipeline components
    std::unique_ptr<IndicatorManager> indicatorManager_;
    std::unique_ptr<SentimentAnalyzer> sentimentAnalyzer_;
    
    // Data queues with validation
    std::queue<MarketDataUpdate> marketDataQueue_;
    std::queue<OrderBookUpdate> orderBookQueue_;
    
    // Threading
    std::thread processingThread_;
    std::atomic<bool> running_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    
    // Configuration
    std::chrono::milliseconds updateInterval_;
    size_t maxQueueSize_;
    
    // Latest processed data
    mutable std::mutex dataMutex_;
    MarketDataUpdate latestMarketData_;
    OrderBookUpdate latestOrderBook_;
    std::unordered_map<std::string, double> latestSentiment_;
    
    // Callbacks
    MarketDataCallback marketDataCallback_;
    OrderBookCallback orderBookCallback_;
    SentimentCallback sentimentCallback_;
    
    // Processing methods
    void processLoop();
    void processMarketData(const MarketDataUpdate& data);
    void processOrderBook(const OrderBookUpdate& data);
    void updateSentiment(const std::string& source, double sentiment);
    
    // Queue management with validation
    template<typename T>
    void pushToQueue(std::queue<T>& queue, const T& data, bool (MarketDataPipeline::*validate)(const T&) const);
    
    template<typename T>
    bool popFromQueue(std::queue<T>& queue, T& data);
    
    // Data quality checks
    bool checkDataFreshness(const std::chrono::system_clock::time_point& timestamp) const;
    bool checkDataConsistency(const OHLCV& data) const;
    bool checkOrderBookConsistency(const OrderBook& data) const;

    DataQualityTracker qualityTracker_;
};

} // namespace novacrypt 