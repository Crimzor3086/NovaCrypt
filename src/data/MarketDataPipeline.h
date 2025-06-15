#pragma once
#include "../indicators/MarketData.h"
#include "../sentiment/SentimentAnalyzer.h"
#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace novacrypt {

class MarketDataPipeline {
public:
    MarketDataPipeline();
    ~MarketDataPipeline();
    
    // Start/Stop the pipeline
    void start();
    void stop();
    
    // Data input methods
    void pushMarketData(const OHLCV& data);
    void pushOrderBook(const OrderBook& orderBook);
    void pushSentiment(const std::string& source, const std::string& text, double score, double confidence);
    
    // Get processed data
    std::vector<double> getLatestFeatures() const;
    OHLCV getLatestMarketData() const;
    OrderBook getLatestOrderBook() const;
    double getLatestSentiment() const;
    
    // Configuration
    void setUpdateInterval(std::chrono::milliseconds interval);
    void setMaxQueueSize(size_t size);

private:
    // Pipeline components
    std::unique_ptr<IndicatorManager> indicatorManager_;
    std::unique_ptr<SentimentAnalyzer> sentimentAnalyzer_;
    
    // Data queues
    std::queue<OHLCV> marketDataQueue_;
    std::queue<OrderBook> orderBookQueue_;
    
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
    OHLCV latestMarketData_;
    OrderBook latestOrderBook_;
    std::vector<double> latestFeatures_;
    
    // Processing methods
    void processLoop();
    void processMarketData();
    void processOrderBook();
    void updateFeatures();
    
    // Queue management
    template<typename T>
    void pushToQueue(std::queue<T>& queue, const T& data);
    
    template<typename T>
    bool popFromQueue(std::queue<T>& queue, T& data);
};

} // namespace novacrypt 