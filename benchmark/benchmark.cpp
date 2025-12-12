#include "../src/OrderBook.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>

using namespace HFT;
using namespace std::chrono;

uint64_t getCurrentTimestamp() {
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

class BenchmarkSuite {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<uint32_t> priceDist;
    std::uniform_int_distribution<uint32_t> qtyDist;
    std::uniform_int_distribution<int> sideDist;

public:
    BenchmarkSuite() : rng(42), priceDist(9900, 10100), qtyDist(1, 1000), sideDist(0, 1) {}
    
    void benchmarkOrderAddition() {
        std::cout << "\n=== Benchmark: Order Addition ===\n";
        OrderBook book;
        
        const int iterations = 100000;
        std::vector<uint64_t> latencies;
        latencies.reserve(iterations);
        
        for (int i = 0; i < iterations; ++i) {
            uint32_t price = priceDist(rng);
            uint32_t qty = qtyDist(rng);
            OrderSide side = (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL;
            
            auto start = high_resolution_clock::now();
            book.addOrder(price, qty, side, getCurrentTimestamp());
            auto end = high_resolution_clock::now();
            
            latencies.push_back(duration_cast<nanoseconds>(end - start).count());
        }
        
        printStatistics(latencies, "Order Addition");
    }
    
    void benchmarkOrderCancellation() {
        std::cout << "\n=== Benchmark: Order Cancellation ===\n";
        OrderBook book;
        
        const int numOrders = 10000;
        std::vector<uint64_t> orderIds;
        
        // Add orders
        for (int i = 0; i < numOrders; ++i) {
            uint64_t id = book.addOrder(priceDist(rng), qtyDist(rng), 
                                         (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL,
                                         getCurrentTimestamp());
            orderIds.push_back(id);
        }
        
        // Cancel orders
        std::vector<uint64_t> latencies;
        latencies.reserve(numOrders);
        
        for (auto orderId : orderIds) {
            auto start = high_resolution_clock::now();
            book.cancelOrder(orderId);
            auto end = high_resolution_clock::now();
            
            latencies.push_back(duration_cast<nanoseconds>(end - start).count());
        }
        
        printStatistics(latencies, "Order Cancellation");
    }
    
    void benchmarkOrderMatching() {
        std::cout << "\n=== Benchmark: Order Matching (Crossing Orders) ===\n";
        OrderBook book;
        
        const int iterations = 10000;
        std::vector<uint64_t> latencies;
        latencies.reserve(iterations);
        
        // Pre-populate order book
        for (int i = 0; i < 1000; ++i) {
            book.addOrder(10000 - i, 100, OrderSide::BUY, getCurrentTimestamp());
            book.addOrder(10100 + i, 100, OrderSide::SELL, getCurrentTimestamp());
        }
        
        // Test matching by crossing spread
        for (int i = 0; i < iterations; ++i) {
            OrderSide side = (i % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
            uint32_t price = (side == OrderSide::BUY) ? 10200 : 9900;
            
            auto start = high_resolution_clock::now();
            book.addOrder(price, 50, side, getCurrentTimestamp());
            auto end = high_resolution_clock::now();
            
            latencies.push_back(duration_cast<nanoseconds>(end - start).count());
        }
        
        printStatistics(latencies, "Order Matching");
        std::cout << "Total trades executed: " << book.getTrades().size() << "\n";
    }
    
    void benchmarkMarketDepthQueries() {
        std::cout << "\n=== Benchmark: Market Depth Queries ===\n";
        OrderBook book;
        
        // Pre-populate
        for (int i = 0; i < 5000; ++i) {
            book.addOrder(priceDist(rng), qtyDist(rng), 
                         (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL,
                         getCurrentTimestamp());
        }
        
        const int iterations = 1000000;
        auto start = high_resolution_clock::now();
        
        volatile uint32_t bestBid, bestAsk;
        volatile int32_t spread;
        
        for (int i = 0; i < iterations; ++i) {
            bestBid = book.getBestBid();
            bestAsk = book.getBestAsk();
            spread = book.getSpread();
        }
        
        auto end = high_resolution_clock::now();
        auto totalNs = duration_cast<nanoseconds>(end - start).count();
        
        std::cout << "Iterations: " << iterations << "\n";
        std::cout << "Total time: " << totalNs / 1000.0 << " microseconds\n";
        std::cout << "Average latency: " << totalNs / iterations << " nanoseconds\n";
        std::cout << "Throughput: " << (iterations * 1e9 / totalNs) << " queries/second\n";
    }

private:
    void printStatistics(const std::vector<uint64_t>& latencies, const std::string& operation) {
        if (latencies.empty()) return;
        
        std::vector<uint64_t> sorted = latencies;
        std::sort(sorted.begin(), sorted.end());
        
        uint64_t sum = 0;
        for (auto lat : sorted) sum += lat;
        
        double mean = static_cast<double>(sum) / sorted.size();
        uint64_t min = sorted.front();
        uint64_t max = sorted.back();
        uint64_t p50 = sorted[sorted.size() / 2];
        uint64_t p95 = sorted[static_cast<size_t>(sorted.size() * 0.95)];
        uint64_t p99 = sorted[static_cast<size_t>(sorted.size() * 0.99)];
        
        std::cout << "\n" << operation << " Statistics:\n";
        std::cout << "  Operations: " << latencies.size() << "\n";
        std::cout << "  Mean:   " << mean << " ns\n";
        std::cout << "  Min:    " << min << " ns\n";
        std::cout << "  P50:    " << p50 << " ns\n";
        std::cout << "  P95:    " << p95 << " ns\n";
        std::cout << "  P99:    " << p99 << " ns\n";
        std::cout << "  Max:    " << max << " ns\n";
        std::cout << "  Throughput: " << (latencies.size() * 1e9 / sum) << " ops/sec\n";
    }
};

int main() {
    std::cout << "===================================\n";
    std::cout << "  HFT Order Book Benchmark Suite  \n";
    std::cout << "===================================\n";
    
    BenchmarkSuite suite;
    
    suite.benchmarkOrderAddition();
    suite.benchmarkOrderCancellation();
    suite.benchmarkOrderMatching();
    suite.benchmarkMarketDepthQueries();
    
    std::cout << "\n=== Benchmark Complete ===\n";
    
    return 0;
}
