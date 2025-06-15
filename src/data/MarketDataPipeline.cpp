#include "MarketDataPipeline.h"
#include <chrono>
#include <algorithm>

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

void MarketDataPipeline::pushMarketData(const OHLCV& data) {
    pushToQueue(marketDataQueue_, data);
}

void MarketDataPipeline::pushOrderBook(const OrderBook& orderBook) {
    pushToQueue(orderBookQueue_, orderBook);
}

void MarketDataPipeline::pushSentiment(const std::string& source,
                                     const std::string& text,
                                     double score,
                                     double confidence) {
    if (source == "Twitter") {
        sentimentAnalyzer_->updateTwitterSentiment(text, score, confidence);
    } else if (source == "Reddit") {
        sentimentAnalyzer_->updateRedditSentiment(text, score, confidence);
    } else if (source == "News") {
        sentimentAnalyzer_->updateNewsSentiment(text, score, confidence);
    }
}

std::vector<double> MarketDataPipeline::getLatestFeatures() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestFeatures_;
}

OHLCV MarketDataPipeline::getLatestMarketData() const {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return latestMarketData_;
}

OrderBook MarketDataPipeline::getLatestOrderBook() const {
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
    OHLCV data;
    while (popFromQueue(marketDataQueue_, data)) {
        indicatorManager_->update(data);
        
        std::lock_guard<std::mutex> lock(dataMutex_);
        latestMarketData_ = data;
    }
}

void MarketDataPipeline::processOrderBook() {
    OrderBook orderBook;
    while (popFromQueue(orderBookQueue_, orderBook)) {
        indicatorManager_->updateOrderBook(orderBook);
        
        std::lock_guard<std::mutex> lock(dataMutex_);
        latestOrderBook_ = orderBook;
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
void MarketDataPipeline::pushToQueue(std::queue<T>& queue, const T& data) {
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

} // namespace novacrypt 