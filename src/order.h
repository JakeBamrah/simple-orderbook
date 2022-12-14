#include <memory>


#ifndef ORDER_H
#define ORDER_H

/*
* Contains all the information of a simple order.
* Each Order is represented as a node within a linked list.
*/
struct Order {
public:
    std::shared_ptr<Order> next_order{nullptr};
    std::shared_ptr<Order> prev_order{nullptr};

    Order(uint64_t id, uint64_t created_at, bool is_bid, uint64_t quantity, uint64_t filled_quantity, uint64_t price);

    // copy constructor
    Order(const Order& o);
    Order& operator=(const Order& o);

    // move constructor
    Order(Order&& o);
    Order& operator=(Order&& o);

    virtual ~Order();

    uint64_t id() const { return _id; };
    uint64_t open_quantity() const;
    uint64_t created_at() const { return _created_at; };
    bool is_bid() const { return _is_bid; };
    uint64_t quantity() const { return _quantity; };
    uint64_t filled_quantity() const { return _filled_quantity; };
    uint64_t filled_cost() const { return _filled_cost; };
    uint64_t price() const { return _price; };

    void fill(uint64_t fill_quantity, uint64_t cost, uint64_t fill_id);

private:
    uint64_t _id;
    uint64_t _created_at;
    bool _is_bid;
    uint64_t _quantity;
    uint64_t _filled_quantity;
    uint64_t _filled_cost{0};
    uint64_t _price;
};

std::ostream& operator<<(std::ostream& os, const Order& o);

#endif
