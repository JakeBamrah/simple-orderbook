#include <iostream>
#include <memory>
#include <chrono>
#include <functional>
#include <cmath>
#include <iomanip>

#include "orderbook.h"
#include "order.cc"
#include "limit.cc"


using std::shared_ptr;
using std::chrono::milliseconds;


uint8_t __MAX_TICK_SIZE__{8};

OrderBook::OrderBook()
    :tick_size{2}
{
    exp = pow(10, tick_size);
}

OrderBook::OrderBook(uint tick_size)
    :tick_size(tick_size)
{
    if (tick_size > __MAX_TICK_SIZE__)
    {
        throw std::invalid_argument("Tick size too large. Must be [0, 8].");
    }

    exp = pow(10, tick_size);
}


std::uint64_t OrderBook::formatLevelPrice(double price)
{
    return (uint64_t)round(price * exp);
}

std::string OrderBook::formatDisplayPrice(double price)
{
    std::stringstream display_price;
    display_price << std::setprecision(tick_size) << std::fixed << price;
    return display_price.str();
}


uint64_t OrderBook::getInsideBid()
{
    if (highest_bid_limit == nullptr)
    {
        return 0;
    }
    return highest_bid_limit->price;
}

double OrderBook::getInsideBidSize()
{
    if (highest_bid_limit == nullptr || highest_bid_limit->head_order == nullptr)
    {
        return 0;
    }
    return highest_bid_limit->head_order->open_quantity();
}


uint64_t OrderBook::getInsideAsk()
{
    if (lowest_ask_limit == nullptr)
    {
        return 0;
    }
    return lowest_ask_limit->price;
}

double OrderBook::getInsideAskSize()
{
    if (lowest_ask_limit == nullptr || lowest_ask_limit->head_order == nullptr)
    {
        return 0;
    }
    return lowest_ask_limit->head_order->open_quantity();
}


shared_ptr<Limit> OrderBook::createBidLimit(uint64_t price)
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

shared_ptr<Limit> OrderBook::createAskLimit(uint64_t price)
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

shared_ptr<Limit> OrderBook::getLimit(QuoteType quote_type, uint64_t price)
{
    shared_ptr<Limit> limit;
    if (quote_type == QuoteType::BID)
    {
        if (bid_limit_map.find(price) == bid_limit_map.end())
        {
            limit = createBidLimit(price);
            bid_limit_map[price] = limit;
        }
        else {
            limit = bid_limit_map.at(price);
        }
    }

    if (quote_type == QuoteType::ASK)
    {
        if (ask_limit_map.find(price) == ask_limit_map.end())
        {
            limit = createAskLimit(price);
            ask_limit_map[price] = limit;
        }
        else {
            limit  = ask_limit_map.at(price);
        }
    }
    return limit;
}


uint64_t getTimestamp()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(now);
    return ms.count();
}

Order OrderBook::createOrder(QuoteType quote_type, uint64_t quantity, uint64_t filled_quantity, double price)
{
    // convert price from tick units to pennies
    uint64_t format_price = formatLevelPrice(price);

    // create order and add to the book order map
    uint64_t created_at = getTimestamp();
    uint64_t order_id = next_id++;
    Order order{
                order_id,
                created_at,
                quote_type,
                quantity,
                filled_quantity,
                format_price
            };
    return order;
}

uint64_t OrderBook::addOrder(std::shared_ptr<Order> order)
{
    order_map[order->id] = order;

    // delete order from limit linked list
    shared_ptr<Limit> limit = getLimit(order->quote_type, order->price);
    limit->addOrder(order);
    return order->id;
}

void OrderBook::removeOrder(shared_ptr<Order> order)
{
    shared_ptr<Limit> limit = getLimit(order->quote_type, order->price);
    order_map.erase(order->id);
    limit->removeOrder(order);
    return;
}

shared_ptr<Order> OrderBook::execute(shared_ptr<Order> order)
{
    // clean-up Order linked list pointers, this is the last order in the limit
    // TODO: emit trade object to confirm order execution
    shared_ptr<Limit> limit = getLimit(order->quote_type, order->price);
    removeOrder(order);
    return limit->head_order;
}

void OrderBook::addLimitOrder(Order& order, std::function<bool(uint64_t, uint64_t)> compare)
{

    // find best priced limit—assume / default new order as a bid
    // NOTE: limit must be the opposite side of incoming order to match orders
    auto limit_map = ask_limit_map;
    shared_ptr<Limit> best_limit = lowest_ask_limit;
    if (order.quote_type == QuoteType::ASK)
    {
        best_limit = highest_bid_limit;
        limit_map = bid_limit_map;
    }

    // if no limit and opposing orders are found, create new limit and new order
    if (best_limit == nullptr)
    {
        addOrder(std::make_shared<Order>(order));
        return;
    }

    // iterate through best price limit and match orders with new order
    while (best_limit != nullptr and compare(best_limit->price, order.price))
    {
        shared_ptr<Order> current_order = best_limit->head_order;
        while (current_order != nullptr && order.open_quantity() > 0)
        {
            if (current_order->open_quantity() > order.open_quantity())
            {
                // TODO: emit trade object
                current_order->filled_quantity += order.open_quantity();
                std::cout << "New order fulfilled before creating limit order. \n";
                return;
            }

            if (current_order->open_quantity() <= order.open_quantity())
            {
                order.filled_quantity += current_order->open_quantity();
                current_order = execute(current_order);
            }
        }

        // orders exhausted for this limit, move to next best limit
        if (best_limit->quantity == 0)
        {
            auto empty_limit = best_limit;

            auto next_limit = best_limit->next;
            best_limit = next_limit;
            if (order.quote_type == QuoteType::BID)
            {
                lowest_ask_limit = next_limit;
            }

            if (order.quote_type == QuoteType::ASK)
            {
                highest_bid_limit = next_limit;
            }

            // TODO: delete limit method
            limit_map.erase(empty_limit->price);
        }
    }

    // new limit order was not fulfilled—create new limit order
    if (order.open_quantity() > 0)
    {
        addOrder(std::make_shared<Order>(order));
    }

    return;
}


std::function<bool(uint64_t, uint64_t)> OrderBook::buildCompareCallback(QuoteType quote_type)
{
    // default to compare function used for BID orders
    std::function<bool(uint64_t, uint64_t)> compare;
    compare = [=](uint64_t first, uint64_t second) { return first <= second; };

    if (quote_type == QuoteType::ASK)
    {
        compare = [=](uint64_t first, uint64_t second) { return first >= second; };
    }
    return compare;
}
