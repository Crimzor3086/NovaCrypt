#include "MarketDataPipeline.h"
#include <chrono>
#include <algorithm>
#include <stdexcept>

namespace novacrypt {

MarketDataPipeline::MarketDataPipeline()
    : running_(false),
      updateInterval_(std::chrono::milliseconds(100)),
      maxQueueSize_(1000)
{
    indicatorManager_ = std::make_unique<IndicatorManager>();
    sentimentAnalyzer_ = std::make_unique<SentimentAnalyzer>();
}

MarketDataPipeline::~MarketDataPipeline() {
    stop();
}

void MarketDataPipeline::start() {
    if (running_) return;
    
    running_ = true;
    processingThread_ = std::thread(&MarketDataPipeline::processLoop, this);
}

void MarketDataPipeline::stop() {
    if (!running_) return;
    
    running_ = false;
    queueCondition_.notify_all();
    
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
}

void MarketDataPipeline::pushMarketData(const MarketDataUpdate& update) {
    if (!validateMarketData(update)) {
        throw std::runtime_error("Invalid market data received");
    }
    pushToQueue(marketDataQueue_, update, &MarketDataPipeline::validateMarketData);
}

void MarketDataPipeline::pushOrderBook(const OrderBookUpdate& update) {
    if (!validateOrderBook(update)) {
        throw std::runtime_error("Invalid order book data received");
    }
    pushToQueue(orderBookQueue_, update, &MarketDataPipeline::validateOrderBook);
}

void MarketDataPipeline::pushSentiment(const std::string& source,
                                     const std::string& text,
                                     double score,
                                     double confidence) {
    if (confidence < 0.0 || confidence > 1.0) {
        throw std::runtime_error("Invalid sentiment confidence value");
    }
    
    if (source == "Twitter") {
        sentimentAnalyzer_->updateTwitterSentiment(text, score, confidence);
    } else if (source == "Reddit") {
        sentimentAnalyzer_->updateRedditSentiment(text, score, confidence);
    } else if (source == "News") {
        sentimentAnalyzer_->updateNewsSentiment(text, score, confidence);
    } else {
        throw std::runtime_error("Invalid sentiment source");
    }
    
    if (sentimentCallback_) {
        sentimentCallback_(sentimentAnalyzer_->getAggregateSentiment());
    }
}

std::vector<double> MarketDataPipeline::getLatestFeatures() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestFeatures_;
}

MarketDataUpdate MarketDataPipeline::getLatestMarketData() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestMarketData_;
}

OrderBookUpdate MarketDataPipeline::getLatestOrderBook() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestOrderBook_;
}

double MarketDataPipeline::getLatestSentiment() const {
    return sentimentAnalyzer_->getAggregateSentiment();
}

void MarketDataPipeline::setUpdateInterval(std::chrono::milliseconds interval) {
    updateInterval_ = interval;
}

void MarketDataPipeline::setMaxQueueSize(size_t size) {
    maxQueueSize_ = size;
}

bool MarketDataPipeline::validateMarketData(const MarketDataUpdate& data) const {
    if (!checkDataFreshness(data.timestamp)) {
        return false;
    }
    
    if (!checkDataConsistency(data.data)) {
        return false;
    }
    
    if (data.confidence < 0.0 || data.confidence > 1.0) {
        return false;
    }
    
    return true;
}

bool MarketDataPipeline::validateOrderBook(const OrderBookUpdate& data) const {
    if (!checkDataFreshness(data.timestamp)) {
        return false;
    }
    
    if (!checkOrderBookConsistency(data.data)) {
        return false;
    }
    
    if (data.confidence < 0.0 || data.confidence > 1.0) {
        return false;
    }
    
    return true;
}

void MarketDataPipeline::setMarketDataCallback(DataUpdateCallback callback) {
    marketDataCallback_ = std::move(callback);
}

void MarketDataPipeline::setOrderBookCallback(OrderBookUpdateCallback callback) {
    orderBookCallback_ = std::move(callback);
}

void MarketDataPipeline::setSentimentCallback(SentimentUpdateCallback callback) {
    sentimentCallback_ = std::move(callback);
}

void MarketDataPipeline::processLoop() {
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait_for(lock, updateInterval_,
                [this] { return !running_ || !marketDataQueue_.empty() || !orderBookQueue_.empty(); });
        }
        
        if (!running_) break;
        
        processMarketData();
        processOrderBook();
        updateFeatures();
        
        // Clear old sentiment data periodically
        sentimentAnalyzer_->clearOldData(std::chrono::hours(24));
    }
}

void MarketDataPipeline::processMarketData() {
    MarketDataUpdate data;
    while (popFromQueue(marketDataQueue_, data)) {
        indicatorManager_->update(data.data);
        
        std::lock_guard<std::mutex> lock(dataMutex_);
        latestMarketData_ = data;
        
        if (marketDataCallback_) {
            marketDataCallback_(data);
        }
    }
}

void MarketDataPipeline::processOrderBook() {
    OrderBookUpdate orderBook;
    while (popFromQueue(orderBookQueue_, orderBook)) {
        indicatorManager_->updateOrderBook(orderBook.data);
        
        std::lock_guard<std::mutex> lock(dataMutex_);
        latestOrderBook_ = orderBook;
        
        if (orderBookCallback_) {
            orderBookCallback_(orderBook);
        }
    }
}

void MarketDataPipeline::updateFeatures() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    
    // Get technical indicator features
    latestFeatures_ = indicatorManager_->getFeatureVector();
    
    // Add sentiment features
    auto sentimentFeatures = sentimentAnalyzer_->getSentimentFeatures();
    latestFeatures_.insert(latestFeatures_.end(),
                         sentimentFeatures.begin(),
                         sentimentFeatures.end());
}

template<typename T>
void MarketDataPipeline::pushToQueue(std::queue<T>& queue, const T& data, bool (MarketDataPipeline::*validate)(const T&) const) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    if (queue.size() >= maxQueueSize_) {
        queue.pop();  // Remove oldest item if queue is full
    }
    
    queue.push(data);
    queueCondition_.notify_one();
}

template<typename T>
bool MarketDataPipeline::popFromQueue(std::queue<T>& queue, T& data) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    if (queue.empty()) {
        return false;
    }
    
    data = queue.front();
    queue.pop();
    return true;
}

bool MarketDataPipeline::checkDataFreshness(const std::chrono::system_clock::time_point& timestamp) const {
    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::seconds>(now - timestamp);
    return age.count() <= 60; // Data must be less than 60 seconds old
}

bool MarketDataPipeline::checkDataConsistency(const OHLCV& data) const {
    // Check for valid price ranges
    if (data.open <= 0 || data.high <= 0 || data.low <= 0 || data.close <= 0) {
        return false;
    }
    
    // Check for logical price relationships
    if (data.high < data.low || data.high < data.open || data.high < data.close ||
        data.low > data.open || data.low > data.close) {
        return false;
    }
    
    // Check for valid volume
    if (data.volume < 0) {
        return false;
    }
    
    return true;
}

bool MarketDataPipeline::checkOrderBookConsistency(const OrderBook& data) const {
    // Check for valid price levels
    for (const auto& bid : data.bids) {
        if (bid.price <= 0 || bid.quantity <= 0) {
            return false;
        }
    }
    
    for (const auto& ask : data.asks) {
        if (ask.price <= 0 || ask.quantity <= 0) {
            return false;
        }
    }
    
    // Check for logical order book structure
    if (!data.bids.empty() && !data.asks.empty()) {
        if (data.bids.front().price >= data.asks.front().price) {
            return false;
        }
    }
    
    return true;
}

} // namespace novacrypt 