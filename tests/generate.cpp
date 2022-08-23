#include <fstream>


#define __NUM_ORDERS__ 1000000

/*
 * Generates order data (quote type, price and quantity) that is used to create
 * an order. Quantity represents order-quantity and initial remaining quantity.
 * Writing the test data to a file allows for more reproducible testing while
 * also avoiding compiler optimisations which may impact benchmarking.
 */
void generateTestData(int num_orders)
{
    std::ofstream file{"order_data.txt"};
    for (int i = 0; i <= num_orders; i++)
    {
        // multiply by a random float to get a price
        // use +1 to avoid price and quantities of 0
        double price{((rand() % 10) + 1) * 1.34};
        int quantity{((rand() % 10) + 1) * 100};
        int is_buy{(i % 2) == 0};
        file << is_buy << " " << price << " " << quantity << std::endl;
    }
    file.close();

    return;
}

int main(int argc, const char* argv[])
{
    uint64_t num_orders = __NUM_ORDERS__;
    if (argc > 1)
    {
        num_orders = std::atoi(argv[1]);
    }

    generateTestData(num_orders);

    return 0;
}
