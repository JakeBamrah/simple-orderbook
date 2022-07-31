#include <assert.h>
#include "../src/orderbook.h"

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
    orderbook.sendLimitOrder(QuoteType::BID, 1, 80);
    orderbook.sendLimitOrder(QuoteType::BID, 1, 90);
    orderbook.sendLimitOrder(QuoteType::BID, 1, 100);

    assert(orderbook.size() == 3);
    assert(orderbook.getInsideBid() == 100);
}

void testOrderBookAsk()
{
    OrderBook orderbook;
    orderbook.sendLimitOrder(QuoteType::ASK, 1, 80);
    orderbook.sendLimitOrder(QuoteType::ASK, 1, 90);
    orderbook.sendLimitOrder(QuoteType::ASK, 1, 100);

    assert(orderbook.size() == 3);
    assert(orderbook.getInsideAsk() == 80);
}

void testOrderBookLimitExecution()
{
    // test single bid-ask execution
    OrderBook orderbook;
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100);
    orderbook.sendLimitOrder(QuoteType::ASK, 10, 100);

    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);

    // test multiple ask to bid execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 20, 100);
    orderbook.sendLimitOrder(QuoteType::ASK, 10, 100);
    orderbook.sendLimitOrder(QuoteType::ASK, 10, 100);

    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);

    // test multiple bid to ask execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100);
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100);
    orderbook.sendLimitOrder(QuoteType::ASK, 20, 100);

    assert(orderbook.size() == 0);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 0);

    // test partial ask execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 10, 100);
    orderbook.sendLimitOrder(QuoteType::ASK, 20, 100);

    assert(orderbook.size() == 1);
    assert(orderbook.getInsideBid() == 0);
    assert(orderbook.getInsideAsk() == 100);

    // test partial bid execution
    orderbook = OrderBook();
    orderbook.sendLimitOrder(QuoteType::BID, 10, 80);
    orderbook.sendLimitOrder(QuoteType::BID, 10, 90);
    orderbook.sendLimitOrder(QuoteType::BID, 15, 90);
    orderbook.sendLimitOrder(QuoteType::ASK, 40, 100);

    assert(orderbook.size() == 1);
    assert(orderbook.getInsideBid() == 1);
    assert(orderbook.getInsideAsk() == 0);
}

// TODO: write market order and cancel order tests

int main()
{
    testOrderBook();
    testOrderBookBid();
    testOrderBookAsk();
    testOrderBookLimitExecution();
}
