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

namespace novacrypt {

struct MarketDataUpdate {
    OHLCV data;
    std::chrono::system_clock::time_point timestamp;
    std::string source;
    double confidence;
};

struct OrderBookUpdate {
    OrderBook data;
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
    void pushMarketData(const MarketDataUpdate& update);
    void pushOrderBook(const OrderBookUpdate& update);
    void pushSentiment(const std::string& source, const std::string& text, double score, double confidence);
    
    // Get processed data
    std::vector<double> getLatestFeatures() const;
    MarketDataUpdate getLatestMarketData() const;
    OrderBookUpdate getLatestOrderBook() const;
    double getLatestSentiment() const;
    
    // Configuration
    void setUpdateInterval(std::chrono::milliseconds interval);
    void setMaxQueueSize(size_t size);
    
    // Data validation
    bool validateMarketData(const MarketDataUpdate& data) const;
    bool validateOrderBook(const OrderBookUpdate& data) const;
    
    // Callbacks for data updates
    using DataUpdateCallback = std::function<void(const MarketDataUpdate&)>;
    using OrderBookUpdateCallback = std::function<void(const OrderBookUpdate&)>;
    using SentimentUpdateCallback = std::function<void(double)>;
    
    void setMarketDataCallback(DataUpdateCallback callback);
    void setOrderBookCallback(OrderBookUpdateCallback callback);
    void setSentimentCallback(SentimentUpdateCallback callback);

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
    std::vector<double> latestFeatures_;
    
    // Callbacks
    DataUpdateCallback marketDataCallback_;
    OrderBookUpdateCallback orderBookCallback_;
    SentimentUpdateCallback sentimentCallback_;
    
    // Processing methods
    void processLoop();
    void processMarketData();
    void processOrderBook();
    void updateFeatures();
    
    // Queue management with validation
    template<typename T>
    void pushToQueue(std::queue<T>& queue, const T& data, bool (MarketDataPipeline::*validate)(const T&) const);
    
    template<typename T>
    bool popFromQueue(std::queue<T>& queue, T& data);
    
    // Data quality checks
    bool checkDataFreshness(const std::chrono::system_clock::time_point& timestamp) const;
    bool checkDataConsistency(const OHLCV& data) const;
    bool checkOrderBookConsistency(const OrderBook& data) const;
};

} // namespace novacrypt 