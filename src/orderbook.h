#include <memory>
#include <unordered_map>

using std::shared_ptr;
using std::unordered_map;


// TODO: unsure over the use of shared pointers, convenient but unnecessary?

enum QuoteType {
    BID,
    ASK
};

struct Limit;

struct Order {
    /*
     * Contains all the information of a simple order.
     * Each Order is represented as a node within a linked list.
     */
public:
    Order(uint64_t id, uint64_t time, uint size, uint remaining, double price):
    id{id}, time{time}, size{size}, remaining{remaining}, price{price}{};

    uint64_t id;
    uint64_t time;
    uint64_t size;
    uint remaining;
    double price;
    shared_ptr<Order> next_order{nullptr};
    shared_ptr<Order> prev_order{nullptr};
    shared_ptr<Limit> parent_limit{nullptr};
};

struct Limit {
    /* A Book level containing a list of orders at a given price-point. */
public:
    Limit(uint price):
    price{price}{};

    uint price{0};
    uint total_volume{0};
    uint size{0};
    shared_ptr<Order> head_order;
    shared_ptr<Order> tail_order;
};

class OrderBook {
    /*
     * A directory containing levels of bids and asks respectively.
     * Limits are stored via hash maps which themselves contain a linked-list
     * of orders.
     *
     * Direct access to the inside of the book is provided efficient matching.
     */
    void createOrder(QuoteType quote_type, uint size, uint remaining, double price);
    shared_ptr<Order> execute(shared_ptr<Order> order);
    shared_ptr<Limit> createLimit(QuoteType quote_type, uint price);
    unordered_map<uint64_t, shared_ptr<Order>> order_ids;
    unordered_map<uint, shared_ptr<Limit>> sell_limit_map;
    unordered_map<uint, shared_ptr<Limit>> buy_limit_map;

    shared_ptr<Limit> lowest_sell;
    shared_ptr<Limit> highest_buy;
    uint num_orders;

    // creating ids for orders by incrementing with each use
    // TODO: this isn't great, should use something more robust?
    uint64_t next_id{0};
public:
    uint64_t sendLimitOrder(QuoteType quote_type, uint size, double price);
    uint64_t sendMarketOrder(QuoteType quote_type, uint size);
    uint64_t sendCancelOrder(uint64_t id);
    double getInsideBid() const;
    double getInsideAsk() const;
    uint size() { return num_orders; };
};
