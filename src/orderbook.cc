#include <iostream>
#include <memory>
#include <chrono>
#include <functional>

#include "orderbook.h"

using std::shared_ptr;
using std::chrono::milliseconds;


const double OrderBook::getInsideBid()
{
    if (highest_bid_limit == nullptr)
    {
        return 0;
    }
    return highest_bid_limit->price;
}


const double OrderBook::getInsideBidSize()
{
    if (highest_bid_limit == nullptr || highest_bid_limit->head_order == nullptr)
    {
        return 0;
    }
    return highest_bid_limit->head_order->remaining;
}


const double OrderBook::getInsideAsk()
{
    if (lowest_ask_limit == nullptr)
    {
        return 0;
    }
    return lowest_ask_limit->price;
}


const double OrderBook::getInsideAskSize()
{
    if (lowest_ask_limit == nullptr || lowest_ask_limit->head_order == nullptr)
    {
        return 0;
    }
    return lowest_ask_limit->head_order->remaining;
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


shared_ptr<Limit> OrderBook::getLimit(QuoteType quote_type, double price)
{
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
    return limit;
}


/* Get epoch time stamp in milliseconds */
uint64_t getTimestamp()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(now);
    return ms.count();
}


const uint64_t OrderBook::createOrder( QuoteType quote_type, uint size, uint remaining, double price)
{
    // create order and add to the book order map
    uint64_t created_at = getTimestamp();
    uint64_t order_id = next_id++;
    shared_ptr<Order> order = std::make_shared<Order>(
                                    order_id,
                                    created_at,
                                    size,
                                    remaining,
                                    price
                                );

    order_map[order_id] = order;

    // find appropriate limit
    shared_ptr<Limit> limit = getLimit(quote_type, price);

    // update head and tail of limit order linked list
    if (limit->head_order == nullptr)
    {
        limit->head_order = order;
    } else {
        // update pointer of existing tail order
        limit->tail_order->next_order = order;
    }

    order->parent_limit = limit;

    limit->tail_order = order;
    limit->total_volume += remaining;
    limit->size++;

    return order_id;
}


std::function<bool(double, double)> OrderBook::buildCompareCallback(QuoteType quote_type)
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


shared_ptr<Order> OrderBook::execute(shared_ptr<Order> order)
{
    shared_ptr<Limit> parent_limit = order->parent_limit;
    parent_limit->total_volume -= order->remaining;
    parent_limit->size--;

    order->remaining = 0;

    // clean-up Order linked list pointers, this is the last order in the limit
    if (parent_limit->head_order == parent_limit->tail_order)
    {
        parent_limit->head_order = nullptr;
        parent_limit->tail_order = nullptr;
    } else {
        parent_limit->head_order = order->next_order;
    }

    order_map.erase(order->id);

    return parent_limit->head_order;
}


const uint64_t OrderBook::sendLimitOrder( QuoteType quote_type, uint size, double price, std::function<bool(double, double)> compare)
{

    // find best priced limit—assume / default new order as a bid
    // NOTE: limit must be the opposite type of incoming order to match orders
    auto limit_map = ask_limit_map;
    shared_ptr<Limit> best_limit = lowest_ask_limit;
    if (quote_type == QuoteType::ASK)
    {
        best_limit = highest_bid_limit;
        limit_map = bid_limit_map;
    }

    // convert price to pennies
    double format_price = price * 100;

    // if no limit and opposing orders are found, create new limit and new order
    if (best_limit == nullptr)
    {
        uint64_t order_id = createOrder(quote_type, size, size, format_price);
        return order_id;
    }

    uint remaining = size;
    while (best_limit != nullptr and compare(best_limit->price, format_price))
    {
        shared_ptr<Order> current_order = best_limit->head_order;
        while (current_order != nullptr && remaining > 0)
        {
            if (current_order->remaining > remaining)
            {
                current_order->remaining -= remaining;
                remaining = 0;
            }

            if (current_order->remaining <= remaining)
            {
                remaining -= current_order->remaining;
                current_order = execute(current_order);
            }
        }

        // orders exhausted for this limit, move to next best limit
        if (best_limit->size == 0)
        {
            limit_map.erase(best_limit->price);
            auto next_limit = best_limit->next;
            best_limit = next_limit;

            if (quote_type == QuoteType::BID)
            {
                lowest_ask_limit = next_limit;
            }

            if (quote_type == QuoteType::ASK)
            {
                highest_bid_limit = next_limit;
            }
        }

        uint64_t order_id = 0;
        if (remaining == 0)
        {
            std::cout << "New order fulfilled before creating limit order. \n";
            return order_id;
        }
    }

    // new limit order was not fulfilled—create new limit order
    uint64_t order_id = 0;
    if (remaining > 0)
    {
        uint64_t o_id = createOrder(quote_type, size, remaining, format_price);
    }

    return order_id;
}


// TODO: sendMarketOrder, cancelOrder
