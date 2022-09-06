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
    std::shared_ptr<Order> head_order{nullptr};
    std::shared_ptr<Order> tail_order{nullptr};
    Limit* next{nullptr};

    Limit(uint64_t price=0);

    Limit(const Limit& l);
    Limit& operator=(const Limit& l);

    Limit(Limit&& l);
    Limit& operator=(Limit&& l);

    virtual ~Limit();

    void removeOrder(std::shared_ptr<Order> order);
    void addOrder(std::shared_ptr<Order> order);

    uint size() const { return _size; };
    uint total_volume() const { return _total_volume; };
    uint64_t price() const { return _price; };

private:
    uint64_t _price;
    uint _total_volume{0};
    uint _size{0};
};

std::ostream& operator<<(std::ostream& os, const Limit& l);

#endif
