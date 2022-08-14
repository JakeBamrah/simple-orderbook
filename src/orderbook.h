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
    Order(uint64_t id, uint64_t created_at, QuoteType quote_type, uint64_t size, uint64_t remaining, uint64_t price);

    Order(Order& o);
    Order& operator=(Order& o);

    Order(Order&& o);
    Order& operator=(Order&& o);

    ~Order();

    uint64_t id;
    uint64_t created_at;
    QuoteType quote_type;
    uint64_t size;
    uint64_t remaining;
    uint64_t price;
    shared_ptr<Order> next_order{nullptr};
    shared_ptr<Order> prev_order{nullptr};
};

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

    void removeOrder(shared_ptr<Order> order);
    void addOrder(shared_ptr<Order> order);

    ~Limit();

    uint64_t price;
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
    // use tick size to build exponent for formatting order prices
    uint tick_size;
    double exp;

    /* Creates Limit and updates lowest ask / highest bid */
    shared_ptr<Limit> createBidLimit(uint64_t price);
    shared_ptr<Limit> createAskLimit(uint64_t price);
    shared_ptr<Limit> getLimit(QuoteType quote_type, uint64_t price);

    /*
     * Executes an Order and cleans-up order being executed.
     * Order is removed from Limit metadata and Order linked-list.
     */
    shared_ptr<Order> execute(shared_ptr<Order> order);

    unordered_map<uint, shared_ptr<Limit>> ask_limit_map;
    unordered_map<uint, shared_ptr<Limit>> bid_limit_map;

    unordered_map<uint64_t, shared_ptr<Order>> order_map;

    /* Use price to access limit directly via limit map */
    shared_ptr<Limit> lowest_ask_limit;
    shared_ptr<Limit> highest_bid_limit;

    // start order ids at 1 and reserve 0 for instances where no order created
    uint64_t next_id{1};

    /*
     * Price is assumed to be given as the tick size initially specified.
     * Price is then formatted using the exponent for limit / order or display.
     * Always returns as whole number.
     */
    uint64_t formatLevelPrice(double price);
    std::string formatDisplayPrice(double price);
public:
    OrderBook();
    OrderBook(uint tick_size);

    /* Generates compare function based on QuoteType */
    std::function<bool(uint64_t, uint64_t)> buildCompareCallback(QuoteType quote_type);

    /*
    * Creates an Order using timestamp as the id and adds to limit.
    * An associated Limit is created for the order if it doesn't already exist.
    */
    Order createOrder(QuoteType quote_type, uint64_t size, uint64_t remaining, double price);
    uint64_t addOrder(shared_ptr<Order> order);
    void removeOrder(shared_ptr<Order> order);

    /*
     * Creates an Order and corresponding limit if necessary.
     * Attempts to fulfill incoming Order before creating limit order.
     * Returns Order id if limit order was created and 0 if fulfilled.
     */
    void addLimitOrder(Order& order, std::function<bool(uint64_t, uint64_t)> compare);

    uint64_t sendMarketOrder(QuoteType quote_type, uint size);
    uint64_t sendCancelOrder(uint64_t order_id);
    uint64_t getInsideBid();
    uint64_t getInsideAsk();
    double getInsideBidSize();
    double getInsideAskSize();
    uint size() { return order_map.size(); };
};

/* Get epoch time stamp in milliseconds */
uint64_t getTimestamp();
