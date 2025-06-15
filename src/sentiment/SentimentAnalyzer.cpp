#include "SentimentAnalyzer.h"
#include <algorithm>
#include <numeric>
#include <chrono>

namespace novacrypt {

SentimentAnalyzer::SentimentAnalyzer() {}

void SentimentAnalyzer::updateTwitterSentiment(const std::string& text, double score, double confidence) {
    SentimentData data{
        score,
        confidence,
        "Twitter",
        std::chrono::system_clock::now(),
        text
    };
    twitterData_.push_back(data);
}

void SentimentAnalyzer::updateRedditSentiment(const std::string& text, double score, double confidence) {
    SentimentData data{
        score,
        confidence,
        "Reddit",
        std::chrono::system_clock::now(),
        text
    };
    redditData_.push_back(data);
}

void SentimentAnalyzer::updateNewsSentiment(const std::string& text, double score, double confidence) {
    SentimentData data{
        score,
        confidence,
        "News",
        std::chrono::system_clock::now(),
        text
    };
    newsData_.push_back(data);
}

double SentimentAnalyzer::getAggregateSentiment() const {
    double twitterSentiment = getTwitterSentiment();
    double redditSentiment = getRedditSentiment();
    double newsSentiment = getNewsSentiment();
    
    // Weight the different sources (can be adjusted based on reliability)
    const double twitterWeight = 0.3;
    const double redditWeight = 0.3;
    const double newsWeight = 0.4;
    
    return (twitterSentiment * twitterWeight +
            redditSentiment * redditWeight +
            newsSentiment * newsWeight);
}

double SentimentAnalyzer::getTwitterSentiment() const {
    return calculateWeightedSentiment(twitterData_);
}

double SentimentAnalyzer::getRedditSentiment() const {
    return calculateWeightedSentiment(redditData_);
}

double SentimentAnalyzer::getNewsSentiment() const {
    return calculateWeightedSentiment(newsData_);
}

std::vector<double> SentimentAnalyzer::getSentimentFeatures() const {
    std::vector<double> features;
    
    // Add individual source sentiments
    features.push_back(getTwitterSentiment());
    features.push_back(getRedditSentiment());
    features.push_back(getNewsSentiment());
    
    // Add aggregate sentiment
    features.push_back(getAggregateSentiment());
    
    // Add sentiment momentum (change over time)
    auto recentSentiments = getRecentSentiments(20);
    if (recentSentiments.size() >= 2) {
        double momentum = recentSentiments.back().score - recentSentiments.front().score;
        features.push_back(momentum);
    } else {
        features.push_back(0.0);
    }
    
    return features;
}

std::vector<SentimentData> SentimentAnalyzer::getRecentSentiments(int count) const {
    std::vector<SentimentData> allData;
    allData.insert(allData.end(), twitterData_.begin(), twitterData_.end());
    allData.insert(allData.end(), redditData_.begin(), redditData_.end());
    allData.insert(allData.end(), newsData_.begin(), newsData_.end());
    
    // Sort by timestamp (newest first)
    std::sort(allData.begin(), allData.end(),
              [](const SentimentData& a, const SentimentData& b) {
                  return a.timestamp > b.timestamp;
              });
    
    // Return the most recent 'count' items
    if (allData.size() > count) {
        allData.resize(count);
    }
    
    return allData;
}

void SentimentAnalyzer::clearOldData(std::chrono::hours maxAge) {
    removeOldData(twitterData_, maxAge);
    removeOldData(redditData_, maxAge);
    removeOldData(newsData_, maxAge);
}

double SentimentAnalyzer::calculateWeightedSentiment(const std::vector<SentimentData>& data) const {
    if (data.empty()) return 0.0;
    
    double totalWeight = 0.0;
    double weightedSum = 0.0;
    
    for (const auto& item : data) {
        // Weight by confidence and recency
        auto age = std::chrono::system_clock::now() - item.timestamp;
        double timeWeight = std::exp(-std::chrono::duration<double>(age).count() / 3600.0); // 1-hour decay
        double weight = item.confidence * timeWeight;
        
        weightedSum += item.score * weight;
        totalWeight += weight;
    }
    
    return totalWeight > 0.0 ? weightedSum / totalWeight : 0.0;
}

void SentimentAnalyzer::removeOldData(std::vector<SentimentData>& data, std::chrono::hours maxAge) {
    auto now = std::chrono::system_clock::now();
    data.erase(
        std::remove_if(data.begin(), data.end(),
                      [&](const SentimentData& item) {
                          return (now - item.timestamp) > maxAge;
                      }),
        data.end()
    );
}

} // namespace novacrypt 