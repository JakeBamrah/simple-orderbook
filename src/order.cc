#include "order.h"


Order::Order(uint64_t id, uint64_t created_at, QuoteType quote_type, uint64_t quantity, uint64_t remaining, uint64_t price)
    :id{id},
    created_at{created_at},
    quote_type{quote_type},
    quantity{quantity},
    remaining{remaining},
    price{price}
{}

// copy constructor
Order::Order(Order& o)
    :id{o.id},
    created_at{o.created_at},
    quote_type{o.quote_type},
    quantity{o.quantity},
    remaining{o.remaining},
    price{o.price}
{}

Order& Order::operator=(Order& o)
{
    id = o.id;
    created_at = o.created_at;
    quote_type = o.quote_type;
    quantity = o.quantity;
    remaining = o.remaining;
    price = o.price;
    next_order = o.next_order;
    prev_order = o.prev_order;

    o.id = o.created_at = o.quantity = o.remaining = o.price = 0;
    o.next_order = o.prev_order = nullptr;

    return *this;
}

// move constructor
Order::Order(Order&& o)
    :id{o.id},
    created_at{o.created_at},
    quote_type{o.quote_type},
    quantity{o.quantity},
    remaining{o.remaining},
    price{o.price}
{
    o.id = o.created_at = o.quantity = o.remaining = o.price = 0;
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
        remaining = o.remaining;
        price = o.price;
        next_order = o.next_order;
        prev_order = o.prev_order;

        o.id = o.created_at = o.quantity = o.remaining = o.price = 0;
        o.next_order = o.prev_order = nullptr;
    }

    return *this;
}

Order::~Order()
{
    id = created_at = quantity = remaining = price = 0;
    next_order = prev_order = nullptr;
}
