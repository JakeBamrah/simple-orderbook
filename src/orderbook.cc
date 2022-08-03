#include "orderbook.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <functional>

using std::shared_ptr;
using std::chrono::milliseconds;


/* Get epoch time stamp in milliseconds */
uint64_t getTimestamp()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(now);
    return ms.count();
}

shared_ptr<Limit> OrderBook::createBidLimit(double price)
{
    shared_ptr<Limit> limit = std::make_shared<Limit>(price);
    if (highest_bid_limit == nullptr)
    {
        highest_bid_limit = limit;
        return limit;
    }
    if (price > highest_bid_limit->price)
    {
        limit->next = highest_bid_limit;
        highest_bid_limit = limit;
        return limit;
    }

    // insert limit into correct position within linked list
    auto head = highest_bid_limit;
    while (head->next != nullptr && head->next->price > price)
    {
        head = head->next;
    }
    limit->next = head->next;
    head->next = limit;
    bid_limit_map[price] = limit;
    return limit;
}

shared_ptr<Limit> OrderBook::createAskLimit(double price)
{
    shared_ptr<Limit> limit = std::make_shared<Limit>(price);
    if (lowest_ask_limit == nullptr)
    {
        lowest_ask_limit = limit;
        return limit;
    }
    if (price < lowest_ask_limit->price)
    {
        limit->next = lowest_ask_limit;
        lowest_ask_limit = limit;
        return limit;
    }

    // insert limit into correct position within linked list
    auto head = lowest_ask_limit;
    while (head->next != nullptr && head->next->price < price)
    {
        head = head->next;
    }
    limit->next = head->next;
    head->next = limit;
    ask_limit_map[price] = limit;
    return limit;
}

uint64_t OrderBook::createOrder(QuoteType quote_type, uint size, uint remaining, double price)
{
    // create order and add to the book order map
    uint64_t created_at = getTimestamp();
    uint64_t order_id = created_at;
    shared_ptr<Order> order = std::make_shared<Order>(
                                    order_id,
                                    created_at,
                                    size,
                                    remaining,
                                    price
                                );

    order_map[order_id] = order;

    // find appropriate limit
    shared_ptr<Limit> limit;
    if (quote_type == QuoteType::BID)
    {
        if (bid_limit_map.find(price) == bid_limit_map.end())
        {
            limit = createBidLimit(price);
            bid_limit_map[price] = limit;
        } else {
            limit = bid_limit_map.at(price);
        }
    }

    if (quote_type == QuoteType::ASK)
    {
        if (ask_limit_map.find(price) == ask_limit_map.end())
        {
            limit = createAskLimit(price);
            ask_limit_map[price] = limit;
        } else {
            limit  = ask_limit_map.at(price);
        }
    }

    // update head and tail of limit order linked list
    if (limit->head_order == nullptr)
    {
        limit->head_order = order;
    } else {
        // update pointer of existing tail order
        limit->tail_order->next_order = order;
    }

    limit->tail_order = order;
    limit->total_volume += remaining;
    limit->size++;

    return order_id;
}

std::function<bool(double, double)> OrderBook::generateCompareCallback(QuoteType quote_type)
{
    // default to compare function used for BID orders
    std::function<bool(double, double)> compare;
    compare = [=](double first, double second) { return first <= second; };

    if (quote_type == QuoteType::ASK)
    {
        compare = [=](double first, double second) { return first >= second; };
    }
    return compare;
}

// TODO: sendLimitOrder, sendMarketOrder, cancelOrder, execute

/* Testing */
int main()
{
    OrderBook orderbook{};
    std::function<bool(double, double)> compare;
    compare = orderbook.generateCompareCallback(QuoteType::ASK);
    uint64_t order_id = orderbook.sendLimitOrder(QuoteType::ASK, 20, 20, compare);

    std::cout << order_id << '\n';
    std::cout << orderbook.size() << '\n';
}
