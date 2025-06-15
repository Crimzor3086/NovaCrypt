#include "../data/MarketDataPipeline.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>

using namespace novacrypt;
using namespace std::chrono_literals;

// Helper function to generate random price
double generateRandomPrice(double basePrice, double volatility) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> d(0.0, 1.0);
    return basePrice * (1.0 + d(gen) * volatility);
}

// Helper function to generate random volume
double generateRandomVolume(double baseVolume, double volatility) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> d(0.0, 1.0);
    return std::max(0.0, baseVolume * (1.0 + d(gen) * volatility));
}

// Helper function to generate random confidence
double generateRandomConfidence() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> d(0.8, 1.0);
    return d(gen);
}

// Helper function to generate order book levels
std::vector<OrderBookUpdate::Level> generateOrderBookLevels(double basePrice, int numLevels, bool isBids) {
    std::vector<OrderBookUpdate::Level> levels;
    double priceStep = basePrice * 0.001; // 0.1% price step
    
    for (int i = 0; i < numLevels; ++i) {
        double price = isBids ? 
            basePrice - (i * priceStep) : 
            basePrice + (i * priceStep);
        double volume = generateRandomVolume(100.0, 0.2);
        levels.push_back({price, volume});
    }
    
    return levels;
}

void simulateMarketData(MarketDataPipeline& pipeline, const std::string& source) {
    double basePrice = 50000.0; // Simulating BTC price
    double baseVolume = 100.0;
    
    while (true) {
        try {
            // Generate market data
            MarketDataUpdate marketData;
            marketData.price = generateRandomPrice(basePrice, 0.001);
            marketData.volume = generateRandomVolume(baseVolume, 0.2);
            marketData.timestamp = std::chrono::system_clock::now();
            marketData.source = source;
            marketData.confidence = generateRandomConfidence();
            
            // Generate order book
            OrderBookUpdate orderBook;
            orderBook.bids = generateOrderBookLevels(marketData.price, 10, true);
            orderBook.asks = generateOrderBookLevels(marketData.price, 10, false);
            orderBook.timestamp = marketData.timestamp;
            orderBook.source = source;
            orderBook.confidence = generateRandomConfidence();
            
            // Push data to pipeline
            pipeline.pushMarketData(marketData);
            pipeline.pushOrderBook(orderBook);
            
            // Generate sentiment
            double sentiment = (generateRandomConfidence() - 0.8) * 2.5 - 1.0; // Scale to [-1, 1]
            pipeline.pushSentimentData(source, sentiment);
            
            // Update base price
            basePrice = marketData.price;
            
            // Sleep for a short time
            std::this_thread::sleep_for(100ms);
            
        } catch (const std::exception& e) {
            std::cerr << "Error in " << source << " simulation: " << e.what() << std::endl;
        }
    }
}

void printQualityReport(const MarketDataPipeline& pipeline, const std::string& source) {
    while (true) {
        std::cout << "\033[2J\033[1;1H"; // Clear screen
        std::cout << "Data Quality Report for " << source << "\n";
        std::cout << "==========================\n\n";
        std::cout << pipeline.generateDataQualityReport(source) << std::endl;
        std::this_thread::sleep_for(1s);
    }
}

int main() {
    try {
        MarketDataPipeline pipeline;
        pipeline.start();
        
        // Start data simulation threads
        std::thread binanceThread(simulateMarketData, std::ref(pipeline), "Binance");
        std::thread coinbaseThread(simulateMarketData, std::ref(pipeline), "Coinbase");
        
        // Start quality report thread
        std::thread reportThread(printQualityReport, std::ref(pipeline), "Binance");
        
        // Wait for threads
        binanceThread.join();
        coinbaseThread.join();
        reportThread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 