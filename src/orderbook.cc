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


uint64_t OrderBook::inside_bid_price() const
{
    if (highest_bid_limit == nullptr)
    {
        return 0;
    }
    return highest_bid_limit->price();
}

double OrderBook::inside_bid_quantity() const
{
    if (highest_bid_limit == nullptr || highest_bid_limit->head_order == nullptr)
    {
        return 0;
    }
    return highest_bid_limit->head_order->open_quantity();
}


uint64_t OrderBook::inside_ask_price() const
{
    if (lowest_ask_limit == nullptr)
    {
        return 0;
    }
    return lowest_ask_limit->price();
}

double OrderBook::inside_ask_quantity() const
{
    if (lowest_ask_limit == nullptr || lowest_ask_limit->head_order == nullptr)
    {
        return 0;
    }
    return lowest_ask_limit->head_order->open_quantity();
}


Limit& OrderBook::createBidLimit(uint64_t price)
{
    Limit& limit = bid_limit_map[price] = Limit{price};
    if (highest_bid_limit == nullptr)
    {
        highest_bid_limit = &limit;
        return limit;
    }
    if (price > highest_bid_limit->price())
    {
        limit.next = highest_bid_limit;
        highest_bid_limit = &limit;
        return limit;
    }

    // insert limit into correct position within linked list
    Limit* head = highest_bid_limit;
    while (head->next != nullptr && head->next->price() > price)
    {
        head = head->next;
    }
    limit.next = head->next;
    head->next = &limit;
    return limit;
}

Limit& OrderBook::createAskLimit(uint64_t price)
{
    Limit& limit = ask_limit_map[price] = Limit{price};
    if (lowest_ask_limit == nullptr)
    {
        lowest_ask_limit = &limit;
        return limit;
    }
    if (price < lowest_ask_limit->price())
    {
        limit.next = lowest_ask_limit;
        lowest_ask_limit = &limit;
        return limit;
    }

    // insert limit into correct position within limit linked list
    Limit* head = lowest_ask_limit;
    while (head->next != nullptr && head->next->price() < price)
    {
        head = head->next;
    }
    limit.next = head->next;
    head->next = &limit;
    return limit;
}

Limit& OrderBook::getLimit(bool is_bid, uint64_t price)
{
    if (is_bid)
    {
        if (bid_limit_map.find(price) == bid_limit_map.end())
        {
            createBidLimit(price);
        }

        return bid_limit_map.at(price);
    } else {
        if (ask_limit_map.find(price) == ask_limit_map.end())
        {
            createAskLimit(price);
        }

        return ask_limit_map.at(price);
    }
}


uint64_t getTimestamp()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    milliseconds ms = std::chrono::duration_cast<milliseconds>(now);
    return ms.count();
}

Order OrderBook::createOrder(bool is_bid, uint64_t quantity, uint64_t filled_quantity, double price)
{
    // convert price from tick units to pennies
    uint64_t format_price = formatLevelPrice(price);

    // create order and add to the book order map
    uint64_t created_at = getTimestamp();
    uint64_t order_id = next_id++;
    Order order{
                order_id,
                created_at,
                is_bid,
                quantity,
                filled_quantity,
                format_price
            };
    return order;
}

void OrderBook::removeOrder(shared_ptr<Order> order)
{
    Limit& limit = getLimit(order->is_bid(), order->price());
    limit.removeOrder(order);
    _size--;
    return;
}

bool OrderBook::matchOrder(Limit* limit, LimitMap limit_map, Order& order)
{
    // iterate through best price limit and match orders with new order
    CompareCallback compare = buildCompareCallback(order.is_bid());
    while (limit != nullptr && compare(limit->price(), order.price()))
    {
        if (order.open_quantity() == 0)
            break;

        shared_ptr<Order> current_order = limit->head_order;
        while (current_order != nullptr)
        {
            uint64_t price = order.is_bid() ? current_order->price() : order.price();
            if (current_order->open_quantity() > order.open_quantity())
            {
                uint64_t cost = price * order.open_quantity();

                // NOTE: fill() call-order matters—quantity will change
                current_order->fill(order.open_quantity(), cost, fill_id++);
                order.fill(order.open_quantity(), cost, fill_id);
                break;
            }

            if (current_order->open_quantity() <= order.open_quantity())
            {
                uint64_t cost = price * current_order->open_quantity();

                // NOTE: fill() call-order matters—quantity will change
                order.fill(current_order->open_quantity(), cost, fill_id++);
                current_order->fill(current_order->open_quantity(), cost, fill_id);

                // remove current order from book and return next order
                removeOrder(current_order);
                current_order = limit->head_order;
            }
        }


        // orders exhausted for this limit, move to next best limit
        if (limit->size() == 0)
        {
            Limit* empty_limit = limit;

            Limit* next_limit = limit->next;
            limit = next_limit;
            if (order.is_bid())
            {
                lowest_ask_limit = next_limit;
            }
            else {
                highest_bid_limit = next_limit;
            }

            limit_map.erase(empty_limit->price());
        }
    }

    return (order.open_quantity() == 0);
}

void OrderBook::addOrder(Order& order)
{
    // find best priced limit—assume / default new order as a bid
    // NOTE: limit must be the opposite side of incoming order to match orders
    LimitMap limit_map = ask_limit_map;
    Limit* best_limit = lowest_ask_limit;
    if (!order.is_bid())
    {
        limit_map = bid_limit_map;
        best_limit = highest_bid_limit;
    }

    // if no limit and opposing orders are found, create new limit and new order
    if (best_limit == nullptr)
    {
        Limit& limit = getLimit(order.is_bid(), order.price());
        limit.addOrder(std::make_shared<Order>(order));
        _size++;
        return;
    }

    bool matched = matchOrder(best_limit, limit_map, order);

    // order unfulfilled—add order to limit
    if (!matched)
    {
        Limit& limit = getLimit(order.is_bid(), order.price());
        limit.addOrder(std::make_shared<Order>(order));
        _size++;
    }

    return;
}


OrderBook::CompareCallback OrderBook::buildCompareCallback(bool is_bid)
{
    // default to compare function used for BID orders
    CompareCallback compare = [=](uint64_t first, uint64_t second)
    {
        return is_bid ? first <= second : first >= second;
    };

    return compare;
}


std::ostream& operator<<(std::ostream& os, const OrderBook& ob)
{
    return os << "<OrderBook>{" \
        << "inside_bid_price:" << ob.inside_bid_price() << " " \
        << "inside_ask_price:" << ob.inside_ask_price() << " " \
        << "inside_bid_quantity:" << ob.inside_bid_quantity() << " " \
        << "inside_ask_quantity" << ob.inside_ask_quantity() << " " \
        << "size:" << ob.size()
        << "} \n";
}
