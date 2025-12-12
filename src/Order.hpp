#pragma once

#include <cstdint>
#include <string>

namespace HFT {

enum class OrderSide : uint8_t {
    BUY = 0,
    SELL = 1
};

enum class OrderType : uint8_t {
    LIMIT = 0,
    MARKET = 1
};

enum class OrderStatus : uint8_t {
    NEW = 0,
    PARTIAL_FILL = 1,
    FILLED = 2,
    CANCELLED = 3,
    REJECTED = 4
};

struct Order {
    uint64_t orderId;
    uint64_t timestamp;      // Nanosecond precision
    uint32_t price;          // Price in ticks (e.g., cents)
    uint32_t quantity;
    uint32_t filledQuantity;
    OrderSide side;
    OrderType type;
    OrderStatus status;
    
    Order() : orderId(0), timestamp(0), price(0), quantity(0), 
              filledQuantity(0), side(OrderSide::BUY), 
              type(OrderType::LIMIT), status(OrderStatus::NEW) {}
    
    Order(uint64_t id, uint64_t ts, uint32_t p, uint32_t qty, OrderSide s)
        : orderId(id), timestamp(ts), price(p), quantity(qty),
          filledQuantity(0), side(s), type(OrderType::LIMIT), 
          status(OrderStatus::NEW) {}
    
    uint32_t getRemainingQuantity() const {
        return quantity - filledQuantity;
    }
    
    bool isFilled() const {
        return filledQuantity >= quantity;
    }
    
    void fill(uint32_t qty) {
        filledQuantity += qty;
        if (isFilled()) {
            status = OrderStatus::FILLED;
        } else {
            status = OrderStatus::PARTIAL_FILL;
        }
    }
};

// Trade execution result
struct Trade {
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    uint32_t price;
    uint32_t quantity;
    uint64_t timestamp;
    
    Trade(uint64_t bid, uint64_t sid, uint32_t p, uint32_t qty, uint64_t ts)
        : buyOrderId(bid), sellOrderId(sid), price(p), quantity(qty), timestamp(ts) {}
};

} // namespace HFT
