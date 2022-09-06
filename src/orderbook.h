#include <memory>
#include <unordered_map>
#include <functional>

#include "order.h"
#include "limit.h"


using std::shared_ptr;
using std::unordered_map;

/*
* A directory containing levels of bids and asks respectively.
* Limits are stored via hash maps which themselves contain a linked-list
* of orders. Find a limit using price and the appropriate limit map.
*
* Direct access to the inside of the book is provided efficient matching.
*/
class OrderBook {
public:
    OrderBook();
    OrderBook(uint tick_size);

    /* Generates compare function based on QuoteType */
    std::function<bool(uint64_t, uint64_t)> buildCompareCallback(bool is_bid);

    /*
     * Price is assumed to be given as the tick size initially specified.
     * Price is then formatted using the exponent for limit / order or display.
     * Always returns as whole number.
     */
    uint64_t formatLevelPrice(double price);

    /*
    * Creates an Order using timestamp as the id and adds to limit.
    * An associated Limit is created for the order if it doesn't already exist.
    */
    virtual Order createOrder(bool is_bid, uint64_t quantity, uint64_t filled_quantity, double price);
    virtual void addOrder(shared_ptr<Order> order);
    virtual void removeOrder(shared_ptr<Order> order);

    /*
     * Creates an Order and corresponding limit if necessary.
     * Attempts to fulfill incoming Order before creating limit order.
     * Returns Order id if limit order was created and 0 if fulfilled.
     */
    void addLimitOrder(Order& order, std::function<bool(uint64_t, uint64_t)> compare);

    uint64_t sendMarketOrder(bool is_bid, uint quantity);
    uint64_t sendCancelOrder(uint64_t order_id);

    uint64_t inside_bid_price() const;
    uint64_t inside_ask_price() const;
    double inside_bid_quantity() const;
    double inside_ask_quantity() const;

    uint size() const { return _size; };
private:
    // use tick size to build exponent for formatting order prices
    uint tick_size;
    double exp;
    uint fill_id{0};
    uint _size{0};

    /* Creates Limit and updates lowest ask / highest bid */
    Limit& createBidLimit(uint64_t price);
    Limit& createAskLimit(uint64_t price);
    Limit& getLimit(bool is_bid, uint64_t price);

    unordered_map<uint, Limit> ask_limit_map;
    unordered_map<uint, Limit> bid_limit_map;

    /* Use price to access limit directly via limit map */
    Limit* lowest_ask_limit{nullptr};
    Limit* highest_bid_limit{nullptr};

    // start order ids at 1 and reserve 0 for instances where no order created
    uint64_t next_id{1};

    std::string formatDisplayPrice(double price);
};

/* Get epoch time stamp in milliseconds */
uint64_t getTimestamp();

std::ostream& operator<<(std::ostream& os, const OrderBook& l);
