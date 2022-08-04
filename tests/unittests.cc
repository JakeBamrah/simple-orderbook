#include <assert.h>
#include <functional>

#include "../src/orderbook.cc"

using std::function;


void testOrderBook()
{
    OrderBook orderbook;
    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);
}

void testOrderBookBid()
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    orderbook.sendLimitOrder(QuoteType::BID, 1, 80, bid_compare);
    orderbook.sendLimitOrder(QuoteType::BID, 1, 90, bid_compare);
    orderbook.sendLimitOrder(QuoteType::BID, 1, 100, bid_compare);

    assert(orderbook.size() == 3);
    assert(orderbook.getInsideBid() == 10000);
}

void testOrderBookAsk()
{
    OrderBook orderbook;

    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);

    orderbook.sendLimitOrder(QuoteType::ASK, 1, 80, ask_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 1, 90, ask_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 1, 100, ask_compare);

    assert(orderbook.size() == 3);
    assert(orderbook.getInsideAsk() == 8000);
}

void testOrderBookLimitExecution()
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    // test single bid-ask execution
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100, bid_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 10, 100, ask_compare);

    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);

    // test multiple ask to bid execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 20, 100, bid_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 10, 100, ask_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 10, 100, ask_compare);

    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);

    // test multiple bid to ask execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100, bid_compare);
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100, bid_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 20, 100, ask_compare);

    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);

    // test partial ask execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100, bid_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 20, 100, ask_compare);

    assert(orderbook.size() == 1);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 10000);
    assert(orderbook.getInsideAskSize() == 10);

    // test partial bid execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 10, 80, bid_compare);
    orderbook.sendLimitOrder(QuoteType::BID, 10, 90, bid_compare);
    orderbook.sendLimitOrder(QuoteType::BID, 15, 90, bid_compare);
    orderbook.sendLimitOrder(QuoteType::ASK, 40, 90, ask_compare);

    assert(orderbook.size() == 2);
    assert(orderbook.getInsideBid() == 8000);
    assert(orderbook.getInsideAsk() == 9000);
    assert(orderbook.getInsideAskSize() == 15);
    assert(orderbook.getInsideBidSize() == 10);
}

// TODO: write market order and cancel order tests

int main()
{
    testOrderBook();
    testOrderBookBid();
    testOrderBookAsk();
    testOrderBookLimitExecution();
}
