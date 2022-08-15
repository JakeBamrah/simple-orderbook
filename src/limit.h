#include <memory>

#include "order.h"


#ifndef LIMIT_H
#define LIMIT_H

/*
* A Book level containing a list of orders at a given price-point.
*
* Orders are stored using shared pointers because they are referenced in a
* number of places incl. order map, limit linked lists, etc.
*/
class Limit {
public:
    Limit(uint64_t price=0)
        :price{price}{};

    ~Limit();

    void removeOrder(std::shared_ptr<Order> order);
    void addOrder(std::shared_ptr<Order> order);

    uint64_t price;
    uint total_volume{0};
    uint size{0};
    std::shared_ptr<Order> head_order{nullptr};
    std::shared_ptr<Order> tail_order{nullptr};
    std::shared_ptr<Limit> next{nullptr};
};

#endif
