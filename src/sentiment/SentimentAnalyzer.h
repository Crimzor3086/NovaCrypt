#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_map>

namespace novacrypt {

struct SentimentData {
    double score;  // -1.0 to 1.0 (negative to positive)
    double confidence;
    std::string source;
    std::chrono::system_clock::time_point timestamp;
    std::string text;
};

class SentimentAnalyzer {
public:
    SentimentAnalyzer();
    
    // Update sentiment data from different sources
    void updateTwitterSentiment(const std::string& text, double score, double confidence);
    void updateRedditSentiment(const std::string& text, double score, double confidence);
    void updateNewsSentiment(const std::string& text, double score, double confidence);
    
    // Get aggregated sentiment metrics
    double getAggregateSentiment() const;
    double getTwitterSentiment() const;
    double getRedditSentiment() const;
    double getNewsSentiment() const;
    
    // Get sentiment features for AI model
    std::vector<double> getSentimentFeatures() const;
    
    // Get recent sentiment data
    std::vector<SentimentData> getRecentSentiments(int count = 10) const;
    
    // Clear old sentiment data
    void clearOldData(std::chrono::hours maxAge);

private:
    // Store sentiment data from different sources
    std::vector<SentimentData> twitterData_;
    std::vector<SentimentData> redditData_;
    std::vector<SentimentData> newsData_;
    
    // Calculate weighted average sentiment
    double calculateWeightedSentiment(const std::vector<SentimentData>& data) const;
    
    // Remove old data
    void removeOldData(std::vector<SentimentData>& data, std::chrono::hours maxAge);
};

} // namespace novacrypt 