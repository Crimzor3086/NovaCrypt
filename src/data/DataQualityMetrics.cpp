#include "DataQualityMetrics.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace novacrypt {

DataQualityTracker::DataQualityTracker(size_t historySize)
    : historySize_(historySize)
{
}

void DataQualityTracker::updateMetrics(const std::string& source, const DataQualityMetrics& metrics) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto& sourceMetrics = sourceMetrics_[source];
    updateSourceMetrics(sourceMetrics, metrics);
}

void DataQualityTracker::recordLatency(const std::string& source, std::chrono::milliseconds latency) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto& metrics = sourceMetrics_[source];
    metrics.latencyHistory.push_back(latency);
    if (metrics.latencyHistory.size() > historySize_) {
        metrics.latencyHistory.pop_front();
    }
    calculateMetrics(metrics);
}

void DataQualityTracker::recordDataPoint(const std::string& source, bool isValid) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto& metrics = sourceMetrics_[source];
    metrics.totalDataPoints++;
    if (isValid) {
        metrics.validDataPoints++;
    } else {
        metrics.rejectedDataPoints++;
    }
    calculateMetrics(metrics);
}

void DataQualityTracker::recordPriceAccuracy(const std::string& source, bool isAccurate) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto& metrics = sourceMetrics_[source];
    if (isAccurate) {
        metrics.accuratePricePoints++;
    }
    calculateMetrics(metrics);
}

void DataQualityTracker::recordVolumeAccuracy(const std::string& source, bool isAccurate) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto& metrics = sourceMetrics_[source];
    if (isAccurate) {
        metrics.accurateVolumePoints++;
    }
    calculateMetrics(metrics);
}

void DataQualityTracker::recordOrderBookAccuracy(const std::string& source, bool isAccurate) {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto& metrics = sourceMetrics_[source];
    if (isAccurate) {
        metrics.accurateOrderBookPoints++;
    }
    calculateMetrics(metrics);
}

DataQualityMetrics DataQualityTracker::getLatestMetrics(const std::string& source) const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto it = sourceMetrics_.find(source);
    if (it == sourceMetrics_.end() || it->second.history.empty()) {
        return DataQualityMetrics{};
    }
    return it->second.history.back();
}

std::vector<DataQualityMetrics> DataQualityTracker::getMetricsHistory(const std::string& source) const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto it = sourceMetrics_.find(source);
    if (it == sourceMetrics_.end()) {
        return {};
    }
    return std::vector<DataQualityMetrics>(it->second.history.begin(), it->second.history.end());
}

double DataQualityTracker::getSourceReliability(const std::string& source) const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto it = sourceMetrics_.find(source);
    if (it == sourceMetrics_.end() || it->second.history.empty()) {
        return 0.0;
    }
    return it->second.history.back().sourceReliability;
}

std::string DataQualityTracker::generateQualityReport(const std::string& source) const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    auto it = sourceMetrics_.find(source);
    if (it == sourceMetrics_.end() || it->second.history.empty()) {
        return "No data available for source: " + source;
    }
    
    const auto& metrics = it->second.history.back();
    return formatMetrics(metrics);
}

std::string DataQualityTracker::generateSummaryReport() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    std::stringstream ss;
    ss << "Data Quality Summary Report\n";
    ss << "=========================\n\n";
    
    for (const auto& [source, metrics] : sourceMetrics_) {
        if (metrics.history.empty()) continue;
        
        ss << "Source: " << source << "\n";
        ss << "------------------------\n";
        ss << formatMetrics(metrics.history.back());
        ss << "\n";
    }
    
    return ss.str();
}

void DataQualityTracker::updateSourceMetrics(SourceMetrics& metrics, const DataQualityMetrics& newMetrics) {
    metrics.history.push_back(newMetrics);
    if (metrics.history.size() > historySize_) {
        metrics.history.pop_front();
    }
}

void DataQualityTracker::calculateMetrics(SourceMetrics& metrics) {
    if (metrics.totalDataPoints == 0) return;
    
    DataQualityMetrics newMetrics;
    
    // Calculate latency metrics
    if (!metrics.latencyHistory.empty()) {
        double sum = std::accumulate(metrics.latencyHistory.begin(), metrics.latencyHistory.end(), 0.0,
            [](double acc, const auto& latency) { return acc + latency.count(); });
        newMetrics.averageLatency = sum / metrics.latencyHistory.size();
        
        newMetrics.maxLatency = std::max_element(metrics.latencyHistory.begin(), metrics.latencyHistory.end())->count();
        
        double sq_sum = std::accumulate(metrics.latencyHistory.begin(), metrics.latencyHistory.end(), 0.0,
            [&](double acc, const auto& latency) {
                double diff = latency.count() - newMetrics.averageLatency;
                return acc + diff * diff;
            });
        newMetrics.latencyStdDev = std::sqrt(sq_sum / metrics.latencyHistory.size());
    }
    
    // Calculate completeness metrics
    newMetrics.dataCompleteness = static_cast<double>(metrics.validDataPoints) / metrics.totalDataPoints * 100.0;
    newMetrics.missingDataRate = static_cast<double>(metrics.rejectedDataPoints) / metrics.totalDataPoints * 100.0;
    
    // Calculate accuracy metrics
    newMetrics.priceAccuracy = static_cast<double>(metrics.accuratePricePoints) / metrics.totalDataPoints * 100.0;
    newMetrics.volumeAccuracy = static_cast<double>(metrics.accurateVolumePoints) / metrics.totalDataPoints * 100.0;
    newMetrics.orderBookAccuracy = static_cast<double>(metrics.accurateOrderBookPoints) / metrics.totalDataPoints * 100.0;
    
    // Calculate source reliability
    newMetrics.sourceReliability = (newMetrics.dataCompleteness * 0.3 +
                                  newMetrics.priceAccuracy * 0.3 +
                                  newMetrics.volumeAccuracy * 0.2 +
                                  newMetrics.orderBookAccuracy * 0.2) / 100.0;
    
    newMetrics.totalDataPoints = metrics.totalDataPoints;
    newMetrics.validDataPoints = metrics.validDataPoints;
    newMetrics.rejectedDataPoints = metrics.rejectedDataPoints;
    newMetrics.timestamp = std::chrono::system_clock::now();
    
    metrics.history.push_back(newMetrics);
    if (metrics.history.size() > historySize_) {
        metrics.history.pop_front();
    }
}

std::string DataQualityTracker::formatMetrics(const DataQualityMetrics& metrics) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    
    ss << "Data Quality Metrics:\n";
    ss << "-------------------\n";
    ss << "Timeliness:\n";
    ss << "  Average Latency: " << metrics.averageLatency << " ms\n";
    ss << "  Max Latency: " << metrics.maxLatency << " ms\n";
    ss << "  Latency StdDev: " << metrics.latencyStdDev << " ms\n\n";
    
    ss << "Completeness:\n";
    ss << "  Data Completeness: " << metrics.dataCompleteness << "%\n";
    ss << "  Missing Data Rate: " << metrics.missingDataRate << "%\n\n";
    
    ss << "Accuracy:\n";
    ss << "  Price Accuracy: " << metrics.priceAccuracy << "%\n";
    ss << "  Volume Accuracy: " << metrics.volumeAccuracy << "%\n";
    ss << "  Order Book Accuracy: " << metrics.orderBookAccuracy << "%\n\n";
    
    ss << "Reliability:\n";
    ss << "  Source Reliability: " << (metrics.sourceReliability * 100.0) << "%\n";
    ss << "  Total Data Points: " << metrics.totalDataPoints << "\n";
    ss << "  Valid Data Points: " << metrics.validDataPoints << "\n";
    ss << "  Rejected Data Points: " << metrics.rejectedDataPoints << "\n";
    
    return ss.str();
}

} // namespace novacrypt 