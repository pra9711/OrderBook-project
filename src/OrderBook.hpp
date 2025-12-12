#pragma once

#include "Order.hpp"
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

namespace HFT {

// Price level containing orders at the same price
struct PriceLevel {
    uint32_t price;
    uint32_t totalQuantity;
    std::list<std::shared_ptr<Order>> orders; // FIFO for price-time priority
    
    PriceLevel(uint32_t p) : price(p), totalQuantity(0) {}
    
    void addOrder(std::shared_ptr<Order> order) {
        orders.push_back(order);
        totalQuantity += order->getRemainingQuantity();
    }
    
    void removeOrder(std::shared_ptr<Order> order) {
        totalQuantity -= order->getRemainingQuantity();
        orders.remove(order);
    }
    
    bool isEmpty() const {
        return orders.empty();
    }
};

class OrderBook {
private:
    // Bid side: higher prices first (descending)
    std::map<uint32_t, std::shared_ptr<PriceLevel>, std::greater<uint32_t>> bids;
    
    // Ask side: lower prices first (ascending)
    std::map<uint32_t, std::shared_ptr<PriceLevel>, std::less<uint32_t>> asks;
    
    // Fast order lookup
    std::unordered_map<uint64_t, std::shared_ptr<Order>> orderMap;
    
    // Trade callback
    std::vector<Trade> trades;
    
    uint64_t nextOrderId = 1;

public:
    OrderBook() = default;
    
    // Add a new limit order
    uint64_t addOrder(uint32_t price, uint32_t quantity, OrderSide side, uint64_t timestamp) {
        auto order = std::make_shared<Order>(nextOrderId++, timestamp, price, quantity, side);
        orderMap[order->orderId] = order;
        
        if (side == OrderSide::BUY) {
            addBuyOrder(order);
        } else {
            addSellOrder(order);
        }
        
        return order->orderId;
    }
    
    // Cancel an order
    bool cancelOrder(uint64_t orderId) {
        auto it = orderMap.find(orderId);
        if (it == orderMap.end()) {
            return false;
        }
        
        auto order = it->second;
        if (order->status == OrderStatus::FILLED || order->status == OrderStatus::CANCELLED) {
            return false;
        }
        
        removeOrder(order);
        order->status = OrderStatus::CANCELLED;
        orderMap.erase(orderId);
        
        return true;
    }
    
    // Modify order quantity (cancel and replace)
    bool modifyOrder(uint64_t orderId, uint32_t newQuantity) {
        auto it = orderMap.find(orderId);
        if (it == orderMap.end()) {
            return false;
        }
        
        auto order = it->second;
        uint32_t oldQuantity = order->quantity;
        order->quantity = newQuantity;
        
        // Update price level quantity
        if (order->side == OrderSide::BUY) {
            auto levelIt = bids.find(order->price);
            if (levelIt != bids.end()) {
                levelIt->second->totalQuantity += (newQuantity - oldQuantity);
            }
        } else {
            auto levelIt = asks.find(order->price);
            if (levelIt != asks.end()) {
                levelIt->second->totalQuantity += (newQuantity - oldQuantity);
            }
        }
        
        return true;
    }
    
    // Get best bid price
    uint32_t getBestBid() const {
        return bids.empty() ? 0 : bids.begin()->first;
    }
    
    // Get best ask price
    uint32_t getBestAsk() const {
        return asks.empty() ? 0 : asks.begin()->first;
    }
    
    // Get bid-ask spread
    int32_t getSpread() const {
        if (bids.empty() || asks.empty()) return -1;
        return static_cast<int32_t>(getBestAsk() - getBestBid());
    }
    
    // Get order book depth
    size_t getBidDepth() const { return bids.size(); }
    size_t getAskDepth() const { return asks.size(); }
    
    // Get all trades executed
    const std::vector<Trade>& getTrades() const { return trades; }
    
    // Print order book snapshot
    void printBook(int levels = 5) const {
        std::cout << "\n========== ORDER BOOK ==========\n";
        std::cout << "   ASKS (Sell Orders)\n";
        std::cout << "Price\t\tQuantity\n";
        std::cout << "-----\t\t--------\n";
        
        int count = 0;
        for (auto it = asks.rbegin(); it != asks.rend() && count < levels; ++it, ++count) {
            std::cout << it->first << "\t\t" << it->second->totalQuantity << "\n";
        }
        
        std::cout << "\nSpread: " << getSpread() << "\n\n";
        
        count = 0;
        for (auto it = bids.begin(); it != bids.end() && count < levels; ++it, ++count) {
            std::cout << it->first << "\t\t" << it->second->totalQuantity << "\n";
        }
        
        std::cout << "   BIDS (Buy Orders)\n";
        std::cout << "================================\n";
    }

private:
    void addBuyOrder(std::shared_ptr<Order> order) {
        // Try to match with existing sell orders
        while (!order->isFilled() && !asks.empty()) {
            auto& bestAsk = asks.begin()->second;
            
            // Check if price crosses
            if (order->price < bestAsk->price) {
                break;
            }
            
            // Match orders
            matchOrders(order, bestAsk);
            
            // Remove empty price level
            if (bestAsk->isEmpty()) {
                asks.erase(asks.begin());
            }
        }
        
        // If order not fully filled, add to book
        if (!order->isFilled()) {
            auto& priceLevel = bids[order->price];
            if (!priceLevel) {
                priceLevel = std::make_shared<PriceLevel>(order->price);
            }
            priceLevel->addOrder(order);
        }
    }
    
    void addSellOrder(std::shared_ptr<Order> order) {
        // Try to match with existing buy orders
        while (!order->isFilled() && !bids.empty()) {
            auto& bestBid = bids.begin()->second;
            
            // Check if price crosses
            if (order->price > bestBid->price) {
                break;
            }
            
            // Match orders
            matchOrders(order, bestBid);
            
            // Remove empty price level
            if (bestBid->isEmpty()) {
                bids.erase(bids.begin());
            }
        }
        
        // If order not fully filled, add to book
        if (!order->isFilled()) {
            auto& priceLevel = asks[order->price];
            if (!priceLevel) {
                priceLevel = std::make_shared<PriceLevel>(order->price);
            }
            priceLevel->addOrder(order);
        }
    }
    
    void matchOrders(std::shared_ptr<Order> incomingOrder, std::shared_ptr<PriceLevel> priceLevel) {
        while (!incomingOrder->isFilled() && !priceLevel->isEmpty()) {
            auto restingOrder = priceLevel->orders.front();
            
            uint32_t tradeQty = std::min(incomingOrder->getRemainingQuantity(), 
                                         restingOrder->getRemainingQuantity());
            
            // Execute trade
            incomingOrder->fill(tradeQty);
            restingOrder->fill(tradeQty);
            
            // Record trade
            uint64_t buyId = (incomingOrder->side == OrderSide::BUY) ? incomingOrder->orderId : restingOrder->orderId;
            uint64_t sellId = (incomingOrder->side == OrderSide::SELL) ? incomingOrder->orderId : restingOrder->orderId;
            
            trades.emplace_back(buyId, sellId, restingOrder->price, tradeQty, incomingOrder->timestamp);
            
            // Remove filled order from price level
            if (restingOrder->isFilled()) {
                priceLevel->removeOrder(restingOrder);
                orderMap.erase(restingOrder->orderId);
            }
        }
    }
    
    void removeOrder(std::shared_ptr<Order> order) {
        if (order->side == OrderSide::BUY) {
            auto levelIt = bids.find(order->price);
            if (levelIt != bids.end()) {
                levelIt->second->removeOrder(order);
                if (levelIt->second->isEmpty()) {
                    bids.erase(levelIt);
                }
            }
        } else {
            auto levelIt = asks.find(order->price);
            if (levelIt != asks.end()) {
                levelIt->second->removeOrder(order);
                if (levelIt->second->isEmpty()) {
                    asks.erase(levelIt);
                }
            }
        }
    }
};

} // namespace HFT
