#include <memory>
#include <ostream>

#include "limit.h"

Limit::Limit(uint64_t price)
    :_price{price}{}

Limit::Limit(const Limit& l)
    :head_order{l.head_order},
    tail_order{l.tail_order},
    next{l.next},
    _price{l.price()},
    _total_volume{l.total_volume()},
    _size{l.size()} {}

Limit& Limit::operator=(const Limit& l)
{
    _price = l.price();
    _total_volume = l.total_volume();
    _size = l.size();
    head_order = l.head_order;
    tail_order = l.tail_order;
    next = l.next;

    return *this;
}

Limit::Limit(Limit&& l)
    :head_order{l.head_order},
    tail_order{l.tail_order},
    next{l.next},
    _price{l.price()},
    _total_volume{l.total_volume()},
    _size{l.size()}
{
    l._price = l._total_volume = l._size = 0;
    l.head_order = nullptr;
    l.tail_order = nullptr;
    // BUG: any limits pointing to this one via next will seg-fault.
    l.next = nullptr;
}

Limit& Limit::operator=(Limit&& l)
{
    if (this != &l)
    {
        _price = l.price();
        _total_volume = l.total_volume();
        _size = l.size();
        head_order = l.head_order;
        tail_order = l.tail_order;
        next = l.next;

        l._price = l._total_volume = l._size = 0;
        l.head_order = nullptr;
        l.tail_order = nullptr;
        // BUG: any limits pointing to this one via next will seg-fault.
        l.next = nullptr;
    }

    return *this;
}

Limit::~Limit()
{
    _price = _total_volume = _size = 0;
    head_order = tail_order = nullptr;
    next = nullptr;
}

void Limit::addOrder(std::shared_ptr<Order> order)
{
    // update head and tail of limit order linked list
    if (head_order == nullptr)
    {
        head_order = order;
        tail_order = order;
    }
    else {
        // update pointer of existing tail order
        tail_order->next_order = order;
        order->prev_order = tail_order;
    }

    tail_order = order;
    _total_volume += order->open_quantity();
    _size++;
    return;
}

void Limit::removeOrder(std::shared_ptr<Order> order)
{
    _total_volume -= order->open_quantity();
    _size--;

    if (head_order == tail_order)
    {
        head_order = nullptr;
        tail_order = nullptr;
        return;
    }

    if (order->next_order)
    {
        order->next_order->prev_order = order->prev_order;
    } else {
        // no next order, make previous order the new tail
        tail_order = order->prev_order;
    }

    if (order->prev_order)
    {
        order->prev_order->next_order = order->next_order;
    } else {
        // no previous order, make next order the new head
        head_order = order->next_order;
    }

    return;
}

std::ostream& operator<<(std::ostream& os, const Limit& l)
{
    return os << "<Limit>{" \
        << "price:" << l.price() << " " \
        << "total_volume:" << l.total_volume() << " " \
        << "size:" << l.size()
        << "} \n";
}
