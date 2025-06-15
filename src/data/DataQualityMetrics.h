#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <vector>
#include <cmath>

namespace novacrypt {

struct DataQualityMetrics {
    // Timeliness metrics
    double averageLatency{0.0};  // in milliseconds
    double maxLatency{0.0};      // in milliseconds
    double latencyStdDev{0.0};   // in milliseconds
    
    // Completeness metrics
    double dataCompleteness{0.0};  // percentage
    double missingDataRate{0.0};   // percentage
    
    // Accuracy metrics
    double priceAccuracy{0.0};      // percentage
    double volumeAccuracy{0.0};     // percentage
    double orderBookAccuracy{0.0};  // percentage
    
    // Overall reliability
    double sourceReliability{0.0};  // 0.0 to 1.0
    
    // Raw counts
    size_t totalDataPoints{0};
    size_t validDataPoints{0};
    size_t rejectedDataPoints{0};
    
    // Timestamp of the metrics
    std::chrono::system_clock::time_point timestamp;
};

class DataQualityTracker {
public:
    explicit DataQualityTracker(size_t historySize = 1000);
    
    // Update metrics for a specific data source
    void updateMetrics(const std::string& source, const DataQualityMetrics& metrics);
    
    // Record individual metrics
    void recordLatency(const std::string& source, std::chrono::milliseconds latency);
    void recordDataPoint(const std::string& source, bool isValid);
    void recordPriceAccuracy(const std::string& source, bool isAccurate);
    void recordVolumeAccuracy(const std::string& source, bool isAccurate);
    void recordOrderBookAccuracy(const std::string& source, bool isAccurate);
    
    // Get metrics
    DataQualityMetrics getLatestMetrics(const std::string& source) const;
    std::vector<DataQualityMetrics> getMetricsHistory(const std::string& source) const;
    double getSourceReliability(const std::string& source) const;
    
    // Generate reports
    std::string generateQualityReport(const std::string& source) const;
    std::string generateSummaryReport() const;

private:
    struct SourceMetrics {
        std::deque<DataQualityMetrics> history;
        std::deque<std::chrono::milliseconds> latencyHistory;
        size_t totalDataPoints{0};
        size_t validDataPoints{0};
        size_t rejectedDataPoints{0};
        size_t accuratePricePoints{0};
        size_t accurateVolumePoints{0};
        size_t accurateOrderBookPoints{0};
    };
    
    void updateSourceMetrics(SourceMetrics& metrics, const DataQualityMetrics& newMetrics);
    void calculateMetrics(SourceMetrics& metrics);
    std::string formatMetrics(const DataQualityMetrics& metrics) const;
    
    size_t historySize_;
    std::unordered_map<std::string, SourceMetrics> sourceMetrics_;
    mutable std::mutex metricsMutex_;
};

} // namespace novacrypt 