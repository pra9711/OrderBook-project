# High-Frequency Trading Order Book Engine

A **low-latency, production-grade C++17 limit order book** implementation designed for high-frequency trading systems. Features nanosecond-precision timestamping, price-time priority matching, and comprehensive performance benchmarking.

## 🎯 Features

- **Limit Order Book** with price-time priority matching
- **Order Operations**: Add, Cancel, Modify
- **Matching Engine**: Automatic order matching when prices cross
- **Market Data**: Best Bid/Ask, Spread calculation, Order book depth
- **Performance Optimized**: STL containers, cache-friendly design
- **Nanosecond Timestamping**: High-precision order tracking
- **Comprehensive Benchmarks**: Latency measurements for all operations

## 📊 Technical Highlights for HFT

### Low-Latency Design
- **Data Structures**: `std::map` for sorted price levels, `std::list` for FIFO order queues
- **Fast Lookups**: `unordered_map` for O(1) order ID lookups
- **Zero-Copy**: Smart pointers for order management
- **Compile Optimizations**: O3, LTO, AVX2 instructions

### Key Algorithms
- **Price-Time Priority**: Orders at same price matched by arrival time (FIFO)
- **Efficient Matching**: Immediate matching on order submission
- **Memory Management**: Smart pointers, minimal allocations in hot path

## 🚀 Build Instructions

### Windows (MSVC)
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux/Mac (GCC/Clang)
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

## 🏃 Running

### Demo Application
```powershell
.\build\Release\orderbook_demo.exe
```

### Benchmark Suite
```powershell
.\build\Release\orderbook_benchmark.exe
```

## 📈 Expected Performance

On modern hardware (Intel i7+):
- **Order Addition**: ~100-500 ns
- **Order Cancellation**: ~50-200 ns
- **Order Matching**: ~200-800 ns
- **Market Data Queries**: ~5-20 ns
- **Throughput**: >1M orders/second

## 🔧 Project Structure

```
OrderBookEngine/
├── src/
│   ├── Order.hpp          # Order structures and enums
│   ├── OrderBook.hpp      # Limit order book implementation
│   └── main.cpp           # Demo application
├── benchmark/
│   └── benchmark.cpp      # Performance benchmarking suite
├── CMakeLists.txt         # Build configuration
└── README.md              # This file
```

## 📝 Usage Example

```cpp
#include "OrderBook.hpp"

HFT::OrderBook book;

// Add buy order: price=10000, qty=100
uint64_t orderId = book.addOrder(10000, 100, HFT::OrderSide::BUY, timestamp);

// Add sell order that crosses spread (triggers matching)
book.addOrder(9999, 50, HFT::OrderSide::SELL, timestamp);

// Get market data
uint32_t bestBid = book.getBestBid();
uint32_t bestAsk = book.getBestAsk();
int32_t spread = book.getSpread();

// Cancel order
book.cancelOrder(orderId);

// Get all executed trades
const auto& trades = book.getTrades();
```

## 🎓 Learning Outcomes

This project demonstrates:
- **Order Book Mechanics**: Understanding limit order books and matching engines
- **Low-Latency C++**: Performance-critical code patterns
- **Data Structures**: Efficient container usage for real-time systems
- **Financial Markets**: Market microstructure knowledge
- **Benchmarking**: Proper latency measurement techniques

## 💼  Highlights

**Key Skills Demonstrated:**
- C++17 (STL, smart pointers, modern features)
- Low-latency system design
- Financial market microstructure
- Performance optimization (compiler flags, data structures)
- Algorithm implementation (matching engine)
- Benchmarking & profiling

**Ideal For:**
- High-Frequency Trading firms
- Quantitative Trading roles
- Market Making companies
- Electronic Trading platforms
- Financial technology positions

## 🔄 Future Enhancements

- Lock-free queue implementation
- Memory pool allocator
- FIX protocol integration
- Multi-threading support
- Historical data replay
- Trading strategy backtester



---

**Note**: This is an educational/portfolio project. Production HFT systems require additional features like risk management, network stack optimization, FPGA acceleration, and regulatory compliance.
