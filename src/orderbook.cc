#include "orderbook.h"
#include <iostream>
#include <memory>
#include <chrono>

using std::shared_ptr;
using std::chrono::milliseconds;


uint64_t getTimestamp()
{
    /* Get epoch time stamp in milliseconds */
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(now);
    return ms.count();
}

Limit OrderBook::createLimit(QuoteType quote_type, double price)
{
    Limit limit{price};
    if (quote_type == QuoteType::BID && highest_bid_limit_price < price)
    {
        highest_bid_limit_price = price;
    }

    if (quote_type == QuoteType::ASK && lowest_ask_limit_price > price)
    {
        lowest_ask_limit_price = price;
    }

    // rely on struct's default move constuctor and move assignment
    return limit;
}

uint64_t OrderBook::createOrder(QuoteType quote_type, uint size, uint remaining, double price)
/*
 * Creates an Order using timestamp as the id and adds to limit.
 * An associated Limit is created for the order if it doesn't already exist.
 */
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
    Limit limit;
    if (quote_type == QuoteType::BID)
    {
        if (bid_limit_map.find(price) == bid_limit_map.end())
        {
            limit = createLimit(quote_type, price);
            bid_limit_map[price] = limit;
        } else {
            limit = bid_limit_map.at(price);
        }
    }

    if (quote_type == QuoteType::ASK)
    {
        if (ask_limit_map.find(price) == ask_limit_map.end())
        {
            limit = createLimit(quote_type, price);
            ask_limit_map[price] = limit;
        } else {
            limit  = ask_limit_map.at(price);
        }
    }

    // update head and tail of limit order linked list
    if (limit.head_order == nullptr)
    {
        limit.head_order = order;
    } else {
        // update pointer of existing tail order
        limit.tail_order->next_order = order;
    }

    limit.tail_order = order;
    limit.total_volume += remaining;
    limit.size++;

    return order_id;
}

uint64_t OrderBook::sendLimitOrder(QuoteType quote_type, uint size, double price)
{
    uint64_t order_id = createOrder(quote_type, size, price, price);
    return order_id;
}

// TODO: sendLimitOrder, sendMarketOrder, cancelOrder, execute

int main()
/* Testing */
{
    OrderBook orderbook{};
    uint64_t order_id = orderbook.sendLimitOrder(QuoteType::ASK, 20, 20);
    std::cout << order_id << '\n';
    std::cout << orderbook.size() << '\n';
}
