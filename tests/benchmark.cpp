#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>
#include <type_traits>
#include <assert.h>
#include <memory>

#include "../src/orderbook.cc"


#define __NUM_ORDERS__ 200000
#define durationMs(a) std::chrono::duration_cast<std::chrono::milliseconds>(a);

/*
 * Splits each line of order data into is_buy, price and quantity.
 * is_buy = 1 = QuoteType::BID
 */
template<typename T>
std::vector<T> split(std::string& s, char delim)
{
    assert(std::is_arithmetic<T>::value);

    std::vector<T> res;
    std::istringstream iss(s);
    std::string value;
    while (std::getline(iss, value, delim))
    {
        res.push_back(std::atof(value.c_str()));
    }
    return res;
}

/* Pull in order data from generated file and convert to Order objects. */
Order** getOrdersFromFile(OrderBook& orderbook)
{
    Order** orders = new Order*[__NUM_ORDERS__];

    uint i = 0;
    std::string line;
    std::ifstream file{"order_data.txt"};
    while (std::getline(file, line) && i <= __NUM_ORDERS__)
    {
        // is_buy, price, quantity
        std::vector<double> values = split<double>(line, ' ');
        QuoteType quote_type = values[0] == 1 ? QuoteType::BID : QuoteType::ASK;
        double price = values[1];
        uint quantity = values[2];
        orders[i] = new Order{
            i, i,
            QuoteType::BID,
            quantity,
            0,
            orderbook.formatLevelPrice(price)
        };
        i++;
    }
    file.close();

    return orders;
}

int run_test(OrderBook& orderbook, Order** orders)
{
    auto start = std::chrono::steady_clock::now();

    int i = 0;
    for (int i; i < __NUM_ORDERS__; i++)
    {
        Order order = **orders;
        auto compare = orderbook.buildCompareCallback(order.quote_type);
        orderbook.addLimitOrder(order, compare);
        orders++;
        if (*orders == nullptr)
        {
            return -1;
        }
    }

    auto end = std::chrono::steady_clock::now();
    auto dur = durationMs(end - start);

    return dur.count();
}

int main(int argc, const char* argv[])
{
    OrderBook orderbook{2};
    Order** orders = getOrdersFromFile(orderbook);
    auto dur = run_test(orderbook, orders);
    std::cout << "Time take: " << dur << "ms/n";

    // clean-up :)
    for (uint i; i <= __NUM_ORDERS__; i++)
    {
        delete orders[i];
    }
    delete [] orders;

    return 0;
}

