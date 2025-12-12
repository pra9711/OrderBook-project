#include "OrderBook.hpp"
#include <iostream>
#include <chrono>

using namespace HFT;
using namespace std::chrono;

uint64_t getCurrentTimestamp() {
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

void demonstrateOrderBook() {
    std::cout << "=== High-Frequency Trading Order Book Demo ===\n\n";
    
    OrderBook book;
    
    std::cout << "1. Adding initial orders...\n";
    
    // Add buy orders
    book.addOrder(10050, 100, OrderSide::BUY, getCurrentTimestamp());
    book.addOrder(10048, 200, OrderSide::BUY, getCurrentTimestamp());
    book.addOrder(10045, 150, OrderSide::BUY, getCurrentTimestamp());
    
    // Add sell orders
    book.addOrder(10055, 120, OrderSide::SELL, getCurrentTimestamp());
    book.addOrder(10058, 180, OrderSide::SELL, getCurrentTimestamp());
    book.addOrder(10060, 100, OrderSide::SELL, getCurrentTimestamp());
    
    book.printBook(5);
    
    std::cout << "\n2. Adding a buy order that crosses the spread (market taker)...\n";
    book.addOrder(10056, 150, OrderSide::BUY, getCurrentTimestamp());
    
    book.printBook(5);
    
    std::cout << "\nTrades executed:\n";
    for (const auto& trade : book.getTrades()) {
        std::cout << "  Buy Order #" << trade.buyOrderId 
                  << " x Sell Order #" << trade.sellOrderId
                  << " | Price: " << trade.price 
                  << " | Qty: " << trade.quantity << "\n";
    }
    
    std::cout << "\n3. Cancelling an order...\n";
    if (book.cancelOrder(5)) {
        std::cout << "  Order #5 cancelled successfully\n";
    }
    
    book.printBook(5);
    
    std::cout << "\n4. Adding aggressive sell order...\n";
    book.addOrder(10047, 250, OrderSide::SELL, getCurrentTimestamp());
    
    book.printBook(5);
    
    std::cout << "\n=== Statistics ===\n";
    std::cout << "Best Bid: " << book.getBestBid() << "\n";
    std::cout << "Best Ask: " << book.getBestAsk() << "\n";
    std::cout << "Spread: " << book.getSpread() << " ticks\n";
    std::cout << "Bid Depth: " << book.getBidDepth() << " levels\n";
    std::cout << "Ask Depth: " << book.getAskDepth() << " levels\n";
    std::cout << "Total Trades: " << book.getTrades().size() << "\n";
}

void performanceTest() {
    std::cout << "\n\n=== Performance Test ===\n";
    
    OrderBook book;
    const int NUM_ORDERS = 10000;
    
    auto start = high_resolution_clock::now();
    
    // Add orders
    for (int i = 0; i < NUM_ORDERS; ++i) {
        uint32_t price = 10000 + (i % 100);
        OrderSide side = (i % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
        book.addOrder(price, 100, side, getCurrentTimestamp());
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(end - start).count();
    
    std::cout << "Added " << NUM_ORDERS << " orders\n";
    std::cout << "Total time: " << duration / 1000.0 << " microseconds\n";
    std::cout << "Average latency per order: " << duration / NUM_ORDERS << " nanoseconds\n";
    std::cout << "Throughput: " << (NUM_ORDERS * 1e9 / duration) << " orders/second\n";
}

int main() {
    demonstrateOrderBook();
    performanceTest();
    
    return 0;
}
