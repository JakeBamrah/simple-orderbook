#include <memory>


#ifndef ORDER_H
#define ORDER_H

enum QuoteType {
    BID,
    ASK
};

/*
* Contains all the information of a simple order.
* Each Order is represented as a node within a linked list.
*/
struct Order {
public:
    Order(uint64_t id, uint64_t created_at, QuoteType quote_type, uint64_t quantity, uint64_t filled_quantity, uint64_t price);

    // copy constructor
    Order(Order& o);
    Order& operator=(Order& o);

    // move constructor
    Order(Order&& o);
    Order& operator=(Order&& o);

    virtual ~Order();

    virtual void fill(uint64_t fill_quantity, uint64_t cost, uint64_t fill_id);

    virtual uint64_t open_quantity();

    uint64_t id;
    uint64_t created_at;
    QuoteType quote_type;
    uint64_t quantity;
    uint64_t filled_quantity;
    uint64_t filled_cost{0};
    uint64_t price;
    std::shared_ptr<Order> next_order{nullptr};
    std::shared_ptr<Order> prev_order{nullptr};
};

#endif
