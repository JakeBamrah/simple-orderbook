#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <vector>
#include <type_traits>
#include <random>
#include <assert.h>
#include <memory>

#include "../src/orderbook.cc"


#define __NUM_ORDERS__ 10000
#define __MIN_RANGE__ 5
#define __MAX_RANGE__ 10
#define durationMs(a) std::chrono::duration_cast<std::chrono::milliseconds>(a);


/*
 * Generate random float within a given range (inclusive).
 */
float getRandomFloat(int min, int max)
{
    // define random number engine see:
    // https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine
    static std::mt19937 gen{1337};
    static std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

/*
 * Generates order data (quote type, price and quantity) that is used to create
 * an order. Writing the test data to a file allows for more reproducible
 * testing while avoiding compiler optimisations which may impact benchmarking.
 */
void generateTestData(int num_orders, int min_range, int max_range)
{
    std::ofstream file{"order_data.txt"};
    for (int i = 0; i <= num_orders; i++)
    {
        // use +1 to avoid price and quantities of 0
        float price = getRandomFloat(min_range, max_range);
        int quantity{((rand() % 10) + 1) * 100};
        int is_buy{(i % 2) == 0};
        file << is_buy << " ";
        file << std::fixed << std::setprecision(2) << price << " ";
        file << quantity << '\n';
    }
    file.close();

    return;
}

/*
 * Splits each line of order data into is_buy, price and quantity.
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
Order** getOrdersFromFile(OrderBook& orderbook, int num_orders, int min_range, int max_range)
{
    Order** orders = new Order*[num_orders + 1];

    uint i = 0;
    std::string line;
    std::ifstream file{"order_data.txt"};
    if (!file.is_open())
    {
        std::cout << "File does not exist. Generating order data! \n";
        generateTestData(num_orders, min_range, max_range);
        file.open("order_data.txt");
    }

    while (std::getline(file, line) && i <= num_orders)
    {
        // is_buy, price, quantity
        std::vector<double> values = split<double>(line, ' ');
        bool is_bid = values[0] == 1 ? true : false;
        double price = values[1];
        uint quantity = values[2];
        orders[i] = new Order{
            i, i,
            is_bid,
            quantity,
            0,
            orderbook.formatLevelPrice(price)
        };
        i++;
    }
    file.close();

    return orders;
}

int run_test(OrderBook& orderbook, Order** orders, int num_orders)
{
    auto start = std::chrono::steady_clock::now();

    for (int i; i < num_orders; i++)
    {
        Order order = **orders;
        orderbook.addOrder(order);
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
    int num_orders = __NUM_ORDERS__;
    int min_range = __MIN_RANGE__;
    int max_range = __MAX_RANGE__;
    if (argc > 1) {
        for (int i = 1; i <= argc; i++) {
            if (i == 1)
                num_orders = std::atoi(argv[1]);
            else if (i == 2)
                min_range = std::atoi(argv[2]);
            else
                max_range = std::atoi(argv[3]);
        }
    }

    if (min_range == 0 || max_range == 0 || min_range > max_range)
        std::cerr << "Min must be less max range and a positive number\n";

    OrderBook orderbook{2};
    Order** orders = getOrdersFromFile(orderbook, num_orders, min_range, max_range);
    auto dur = run_test(orderbook, orders, num_orders);
    std::cout << "Time take: " << dur << "ms \n";

    // clean-up :)
    for (uint i; i <= num_orders; i++)
    {
        delete orders[i];
    }
    delete [] orders;

    return 0;
}
