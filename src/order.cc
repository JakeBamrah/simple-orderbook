#include "order.h"


Order::Order(uint64_t id, uint64_t created_at, QuoteType quote_type, uint64_t quantity, uint64_t filled_quantity, uint64_t price)
    :id{id},
    created_at{created_at},
    quote_type{quote_type},
    quantity{quantity},
    filled_quantity{filled_quantity},
    price{price}
{}

// copy constructor
Order::Order(Order& o)
    :id{o.id},
    created_at{o.created_at},
    quote_type{o.quote_type},
    quantity{o.quantity},
    filled_quantity{o.filled_quantity},
    price{o.price}
{}

Order& Order::operator=(Order& o)
{
    id = o.id;
    created_at = o.created_at;
    quote_type = o.quote_type;
    quantity = o.quantity;
    filled_quantity = o.filled_quantity;
    price = o.price;
    next_order = o.next_order;
    prev_order = o.prev_order;

    return *this;
}

// move constructor
Order::Order(Order&& o)
    :id{o.id},
    created_at{o.created_at},
    quote_type{o.quote_type},
    quantity{o.quantity},
    filled_quantity{o.filled_quantity},
    price{o.price}
{
    o.id = o.created_at = o.quantity = o.filled_quantity = o.price = 0;
    o.next_order = o.prev_order = nullptr;
}

Order& Order::operator=(Order&& o)
{
    if (this != &o)
    {
        id = o.id;
        created_at = o.created_at;
        quote_type = o.quote_type;
        quantity = o.quantity;
        filled_quantity = o.filled_quantity;
        price = o.price;
        next_order = o.next_order;
        prev_order = o.prev_order;

        o.id = o.created_at = o.quantity = o.filled_quantity = o.price = 0;
        o.next_order = o.prev_order = nullptr;
    }

    return *this;
}

Order::~Order()
{
    id = created_at = quantity = filled_quantity = price = 0;
    next_order = prev_order = nullptr;
}

void Order::fill(uint64_t fill_quantity, uint64_t cost, uint64_t fill_id)
{
    filled_quantity += fill_quantity;
    filled_cost += cost;
}

uint64_t Order::open_quantity()
{
    return quantity - filled_quantity;
}
