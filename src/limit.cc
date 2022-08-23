#include <memory>

#include "limit.h"


Limit::~Limit()
{
    price = total_volume = quantity = 0;
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
    total_volume += order->remaining;
    quantity++;
    return;
}

void Limit::removeOrder(std::shared_ptr<Order> order)
{
    quantity--;
    total_volume -= order->remaining;

    if (head_order == tail_order)
    {
        head_order = nullptr;
        tail_order = nullptr;
    } else {
        head_order = order->next_order;
    }
    return;
}

