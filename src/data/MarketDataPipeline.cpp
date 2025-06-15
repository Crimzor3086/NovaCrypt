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

void MarketDataPipeline::pushMarketData(const MarketDataUpdate& data) {
    if (!validateMarketData(data)) {
        qualityTracker_.recordDataPoint(data.source, false);
        throw std::runtime_error("Invalid market data received");
    }
    auto now = std::chrono::system_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(now - data.timestamp);
    qualityTracker_.recordLatency(data.source, latency);
    qualityTracker_.recordDataPoint(data.source, true);
    pushToQueue(marketDataQueue_, data, marketDataMutex_);
}

void MarketDataPipeline::pushOrderBook(const OrderBookUpdate& data) {
    if (!validateOrderBook(data)) {
        qualityTracker_.recordDataPoint(data.source, false);
        throw std::runtime_error("Invalid order book data received");
    }
    auto now = std::chrono::system_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(now - data.timestamp);
    qualityTracker_.recordLatency(data.source, latency);
    qualityTracker_.recordDataPoint(data.source, true);
    pushToQueue(orderBookQueue_, data, orderBookMutex_);
}

void MarketDataPipeline::pushSentimentData(const std::string& source, double sentiment) {
    qualityTracker_.recordDataPoint(source, true);
    updateSentiment(source, sentiment);
}

MarketDataUpdate MarketDataPipeline::getLatestMarketData() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestMarketData_;
}

OrderBookUpdate MarketDataPipeline::getLatestOrderBook() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestOrderBook_;
}

double MarketDataPipeline::getLatestSentiment(const std::string& source) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    auto it = latestSentiment_.find(source);
    return it != latestSentiment_.end() ? it->second : 0.0;
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
    if (data.price <= 0.0) {
        return false;
    }
    if (data.volume < 0.0) {
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
    if (data.confidence < 0.0 || data.confidence > 1.0) {
        return false;
    }
    if (data.bids.empty() || data.asks.empty()) {
        return false;
    }
    for (size_t i = 1; i < data.bids.size(); ++i) {
        if (data.bids[i].price >= data.bids[i-1].price) {
            return false;
        }
    }
    for (size_t i = 1; i < data.asks.size(); ++i) {
        if (data.asks[i].price <= data.asks[i-1].price) {
            return false;
        }
    }
    if (data.bids.front().price >= data.asks.front().price) {
        return false;
    }
    for (const auto& bid : data.bids) {
        if (bid.volume <= 0.0) {
            return false;
        }
    }
    for (const auto& ask : data.asks) {
        if (ask.volume <= 0.0) {
            return false;
        }
    }
    return true;
}

void MarketDataPipeline::setMarketDataCallback(MarketDataCallback callback) {
    marketDataCallback_ = std::move(callback);
}

void MarketDataPipeline::setOrderBookCallback(OrderBookCallback callback) {
    orderBookCallback_ = std::move(callback);
}

void MarketDataPipeline::setSentimentCallback(SentimentCallback callback) {
    sentimentCallback_ = std::move(callback);
}

DataQualityMetrics MarketDataPipeline::getDataQualityMetrics(const std::string& source) const {
    return qualityTracker_.getLatestMetrics(source);
}

std::string MarketDataPipeline::generateDataQualityReport(const std::string& source) const {
    return qualityTracker_.generateQualityReport(source);
}

std::string MarketDataPipeline::generateDataQualitySummary() const {
    return qualityTracker_.generateSummaryReport();
}

void MarketDataPipeline::processLoop() {
    while (running_) {
        MarketDataUpdate marketData;
        OrderBookUpdate orderBook;
        if (popFromQueue(marketDataQueue_, marketData, marketDataMutex_)) {
            processMarketData(marketData);
        }
        if (popFromQueue(orderBookQueue_, orderBook, orderBookMutex_)) {
            processOrderBook(orderBook);
        }
        std::this_thread::sleep_for(updateInterval_);
    }
}

void MarketDataPipeline::processMarketData(const MarketDataUpdate& data) {
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        latestMarketData_ = data;
    }
    if (marketDataCallback_) {
        marketDataCallback_(data);
    }
    qualityTracker_.recordPriceAccuracy(data.source, data.confidence >= 0.95);
    qualityTracker_.recordVolumeAccuracy(data.source, data.confidence >= 0.90);
}

void MarketDataPipeline::processOrderBook(const OrderBookUpdate& data) {
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        latestOrderBook_ = data;
    }
    if (orderBookCallback_) {
        orderBookCallback_(data);
    }
    qualityTracker_.recordOrderBookAccuracy(data.source, data.confidence >= 0.95);
}

void MarketDataPipeline::updateSentiment(const std::string& source, double sentiment) {
    std::lock_guard<std::mutex> lock(dataMutex_);
    latestSentiment_[source] = sentiment;
    if (sentimentCallback_) {
        sentimentCallback_(source, sentiment);
    }
}

template<typename T>
void MarketDataPipeline::pushToQueue(std::queue<T>& queue, const T& data, std::mutex& mutex) {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.size() >= maxQueueSize_) {
        queue.pop();
    }
    queue.push(data);
    queueCondition_.notify_one();
}

template<typename T>
bool MarketDataPipeline::popFromQueue(std::queue<T>& queue, T& data, std::mutex& mutex) {
    std::lock_guard<std::mutex> lock(mutex);
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
    return age.count() <= 60;
}

} // namespace novacrypt 