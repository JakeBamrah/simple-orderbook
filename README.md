## Limit OrderBook

A simple limit orderbook implementation (WIP). At the moment, only limit-orders
are implemented within the orderbook API.

### Orderbook structure
Orderbook comprises of:
- `Limits` stored in a map
- `Orders` stored as doubley-linked lists within each limit

```
    Order
    uint64_t id
    uint64_t open_quantity
    uint64_t created_at
    bool is_bid
    uint64_t quantity
    uint64_t filled_quantity
    uint64_t filled_cost
    uint64_t price
    Order *nextOrder;
    Order *prev_order;

    Limit
    int price;
    int size;
    int total_volume;
    Limit* next;
    Order* headOrder;
    Order* tailOrder;

    OrderBook
    Limit* bidLimits;
    Limit* askLimits;
    Limit* lowestSell;
    Limit* highestBuy;
```

### Unit tests
Unit tests can be ran by:
- Compiling tests by running `./compile` in the project root directory
- Running `cd build && ctest` from root directory

### Benchmarking
Benchmarks can be tested in the `tests` folder. Order data will be generated as
a text file prior to running benchmarks. In-order to re-generate order data, it
must be deleted before compiling and running `benchmark.cpp`.

Benchmark order data is generated within a specified range.

### TODO
- Replace usage of limit `unordered map` with a (sparse?) array
- Remove usage of shared pointers for `Order` objects
- Add market and stop orders to `OrderBook` api

