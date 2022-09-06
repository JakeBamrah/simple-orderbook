#include <ostream>

#include "order.h"


Order::Order(uint64_t id, uint64_t created_at, bool is_bid, uint64_t quantity, uint64_t filled_quantity, uint64_t price)
    :_id{id},
    _created_at{created_at},
    _is_bid{is_bid},
    _quantity{quantity},
    _filled_quantity{filled_quantity},
    _price{price}
{}

// copy constructor
Order::Order(const Order& o)
    :_id{o.id()},
    _created_at{o.created_at()},
    _is_bid{o.is_bid()},
    _quantity{o.quantity()},
    _filled_quantity{o.filled_quantity()},
    _price{o.price()}
{}

Order& Order::operator=(const Order& o)
{
    _id = o.id();
    _created_at = o.created_at();
    _is_bid = o.is_bid();
    _quantity = o.quantity();
    _filled_quantity = o.filled_quantity();
    _price = o.price();
    next_order = o.next_order;
    prev_order = o.prev_order;

    return *this;
}

std::ostream& operator<<(std::ostream& os, const Order& o)
{
    std::string q = o.is_bid() ? "BID" : "ASK";
    return os << "<Order:" << o.id() << ">{" \
        << "id:" << o.id() << " " \
        << "is_bid:" << q << " " \
        << "quantity:" << o.quantity() << " " \
        << "price:" << o.price() << " " \
        << "filled_quantity:" << o.filled_quantity() << " " \
        << "open_quantity:" << o.open_quantity()
        << "} \n";
}

// move constructor
Order::Order(Order&& o)
    :_id{o.id()},
    _created_at{o.created_at()},
    _is_bid{o.is_bid()},
    _quantity{o.quantity()},
    _filled_quantity{o.filled_quantity()},
    _price{o.price()}
{
    o._id = o._created_at = o._quantity = o._filled_quantity = o._price = 0;
    o.next_order = o.prev_order = nullptr;
}

Order& Order::operator=(Order&& o)
{
    if (this != &o)
    {
        _id = o.id();
        _created_at = o.created_at();
        _is_bid = o.is_bid();
        _quantity = o.quantity();
        _filled_quantity = o.filled_quantity();
        _price = o.price();
        next_order = o.next_order;
        prev_order = o.prev_order;

        o._id = o._created_at = o._quantity = o._filled_quantity = o._price = 0;
        o.next_order = o.prev_order = nullptr;
    }

    return *this;
}

Order::~Order()
{
    _id = _created_at = _quantity = _filled_quantity = _price = 0;
    next_order = prev_order = nullptr;
}

void Order::fill(uint64_t fill_quantity, uint64_t cost, uint64_t fill_id)
{
    _filled_quantity += fill_quantity;
    _filled_cost += cost;
}

uint64_t Order::open_quantity() const
{
    return quantity() - filled_quantity();
}
