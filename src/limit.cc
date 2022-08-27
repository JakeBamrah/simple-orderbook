#include <memory>

#include "limit.h"

Limit::Limit(uint64_t price)
    :price{price}{}

Limit::Limit(const Limit& l)
    :price{l.price},
    total_volume{l.total_volume},
    size{l.size},
    head_order{l.head_order},
    tail_order{l.tail_order},
    next{l.next} {}

Limit& Limit::operator=(const Limit& l)
{
    price = l.price;
    total_volume = l.total_volume;
    size = l.size;
    head_order = l.head_order;
    tail_order = l.tail_order;
    next = l.next;

    return *this;
}

Limit::Limit(Limit&& l)
    :price{l.price},
    total_volume{l.total_volume},
    size{l.size},
    head_order{l.head_order},
    tail_order{l.tail_order},
    next{l.next}
{
    l.price = l.total_volume = l.size = 0;
    l.head_order = nullptr;
    l.tail_order = nullptr;
    // BUG: any limits pointing to this one via next will seg-fault.
    l.next = nullptr;
}

Limit& Limit::operator=(Limit&& l)
{
    if (this != &l)
    {
        price = l.price;
        total_volume = l.total_volume;
        size = l.size;
        head_order = l.head_order;
        tail_order = l.tail_order;
        next = l.next;

        l.price = l.total_volume = l.size = 0;
        l.head_order = nullptr;
        l.tail_order = nullptr;
        // BUG: any limits pointing to this one via next will seg-fault.
        l.next = nullptr;
    }

    return *this;
}

Limit::~Limit()
{
    price = total_volume = size = 0;
    head_order = tail_order = nullptr;
    next = nullptr;
}

void Limit::addOrder(std::shared_ptr<Order> order)
{
    // update head and tail of limit order linked list
    if (head_order == nullptr)
    {
        head_order = order;
    }
    else {
        // update pointer of existing tail order
        tail_order->next_order = order;
    }

    tail_order = order;
    total_volume += order->open_quantity();
    size++;
    return;
}

void Limit::removeOrder(std::shared_ptr<Order> order)
{
    if (head_order == tail_order)
    {
        head_order = nullptr;
        tail_order = nullptr;
    } else {
        head_order = order->next_order;
    }

    total_volume -= order->open_quantity();
    size--;
    return;
}
