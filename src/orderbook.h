#include <memory>
#include <unordered_map>
#include <functional>

using std::shared_ptr;
using std::unordered_map;


enum QuoteType {
    BID,
    ASK
};

struct Limit;

/*
* Contains all the information of a simple order.
* Each Order is represented as a node within a linked list.
*/
struct Order {
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
};

/*
* A Book level containing a list of orders at a given price-point.
*
* Orders are stored using shared pointers because they are referenced in a
* number of places incl. order map, limit linked lists, etc.
*/
struct Limit {
public:
    Limit(double price=0)
        :price{price}{};

    double price;
    uint total_volume{0};
    uint size{0};
    shared_ptr<Order> head_order{nullptr};
    shared_ptr<Order> tail_order{nullptr};
    shared_ptr<Limit> next{nullptr};
};


/*
* A directory containing levels of bids and asks respectively.
* Limits are stored via hash maps which themselves contain a linked-list
* of orders. Find a limit using price and the appropriate limit map.
*
* Direct access to the inside of the book is provided efficient matching.
*/
class OrderBook {
    const Limit& getLimit(double price);

    /* Creates Limit and updates lowest ask / highest bid */
    shared_ptr<Limit> createBidLimit(double price);
    shared_ptr<Limit> createAskLimit(double price);

    /*
    * Creates an Order using timestamp as the id and adds to limit.
    * An associated Limit is created for the order if it doesn't already exist.
    */
    uint64_t createOrder(QuoteType quote_type, uint size, uint remaining, double price);
    shared_ptr<Order> execute(shared_ptr<Order> order);
    unordered_map<uint, shared_ptr<Limit>> ask_limit_map;
    unordered_map<uint, shared_ptr<Limit>> bid_limit_map;
    unordered_map<uint64_t, shared_ptr<Order>> order_map;

    // use price to access limit directly via limit map
    shared_ptr<Limit> lowest_ask_limit;
    shared_ptr<Limit> highest_bid_limit;
    uint64_t next_id{0};
public:
    /* Generates compare function based on QuoteType */
    std::function<bool(double, double)> generateCompareCallback(QuoteType quote_type);

    uint64_t sendLimitOrder(QuoteType quote_type, uint size, double price, std::function<bool(double, double)>);
    uint64_t sendMarketOrder(QuoteType quote_type, uint size);
    uint64_t sendCancelOrder(uint64_t order_id);
    double getInsideBid() const { return 0; };
    double getInsideAsk() const { return 0; };
    uint size() { return order_map.size(); };
};
