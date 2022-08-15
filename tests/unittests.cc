#include <assert.h>
#include <functional>
#include <gtest/gtest.h>

#include "../src/orderbook.cc"

using std::function;

// TODO: write market order and cancel order tests

TEST(OrderBookTest, TestOrderBookInitialize)
{
    OrderBook orderbook;
    ASSERT_EQ(orderbook.size(), 0);
    ASSERT_EQ(orderbook.getInsideBid(), 0);
    ASSERT_EQ(orderbook.getInsideAsk(), 0);
}

TEST(OrderBookTest, TestOrderBookInitException)
{
    EXPECT_THROW({
        try {
            OrderBook orderbook{9};
        }
        catch(std::invalid_argument err) {
            EXPECT_STREQ("Tick size too large. Must be [0, 8].", err.what());
            throw;
        }
    }, std::invalid_argument);
}

TEST(OrderBookTest, TestOrderBookDefaultTickSize)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 10, 100.4564);
    orderbook.addLimitOrder(o1, bid_compare);

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.getInsideBid(), 10046);
}

TEST(OrderBookTest, TestOrderBookTickSize)
{
    OrderBook orderbook{4};

    function<bool(double, double)> bid_compare;
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 10, 100.4564);
    orderbook.addLimitOrder(o1, bid_compare);

    std::cout << orderbook.getInsideBid();

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.getInsideBid(), 1004564);
}

TEST(OrderBookTest, TestOrderBookBidCreate)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 1, 1, 80);
    Order o2 = orderbook.createOrder(QuoteType::BID, 1, 1, 90);
    Order o3 = orderbook.createOrder(QuoteType::BID, 1, 1, 100);

    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);

    ASSERT_TRUE(orderbook.size() == 3);
    ASSERT_TRUE(orderbook.getInsideBid() == 10000);
}

TEST(OrderBookTest, TestOrderBookAskCreate)
{
    OrderBook orderbook;

    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 1, 1, 80);
    Order o2 = orderbook.createOrder(QuoteType::ASK, 1, 1, 90);
    Order o3 = orderbook.createOrder(QuoteType::ASK, 1, 1, 100);

    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, ask_compare);
    orderbook.addLimitOrder(o3, ask_compare);

    ASSERT_TRUE(orderbook.size() == 3);
    ASSERT_TRUE(orderbook.getInsideAsk() == 8000);
}

TEST(OrderBookTest, TestSingleBidAskExecute)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 10, 10, 100);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 10, 100);

    // test single bid-ask execution
    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, bid_compare);

    ASSERT_TRUE(orderbook.size() == 0);
    ASSERT_TRUE(orderbook.getInsideBid() == 0);
    ASSERT_TRUE(orderbook.getInsideAsk() == 0);
}

TEST(OrderBookTest, TestMultipleAskBidExecute)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 20, 20, 100);
    Order o2 = orderbook.createOrder(QuoteType::ASK, 10, 10, 100);
    Order o3 = orderbook.createOrder(QuoteType::ASK, 10, 10, 100);

    // test multiple ask to bid execution
    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, ask_compare);
    orderbook.addLimitOrder(o3, ask_compare);

    ASSERT_TRUE(orderbook.size() == 0);
    ASSERT_TRUE(orderbook.getInsideBid() == 0);
    ASSERT_TRUE(orderbook.getInsideAsk() == 0);
}

TEST(OrderBookTest, TestMultipleBidAskExecute)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 20, 20, 100);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 10, 100);
    Order o3 = orderbook.createOrder(QuoteType::BID, 10, 10, 100);

    // test multiple bid to ask execution
    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);

    ASSERT_TRUE(orderbook.size() == 0);
    ASSERT_TRUE(orderbook.getInsideBid() == 0);
    ASSERT_TRUE(orderbook.getInsideAsk() == 0);
}

TEST(OrderBookTest, TestPartialAskExecution)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 20, 20, 100);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 10, 100);

    // test partial ask execution
    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, bid_compare);

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.getInsideBid(), 0);
    ASSERT_EQ(orderbook.getInsideAsk(), 10000);
    ASSERT_EQ(orderbook.getInsideAskSize(), 10);
}

TEST(OrderBookTest, TestPartialBidExecution)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 10, 80);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 10, 90);
    Order o3 = orderbook.createOrder(QuoteType::BID, 15, 15, 90);
    Order o4 = orderbook.createOrder(QuoteType::ASK, 40, 40, 90);

    // test partial bid execution
    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);
    orderbook.addLimitOrder(o4, ask_compare);

    ASSERT_EQ(orderbook.size(), 2);
    ASSERT_EQ(orderbook.getInsideBid(), 8000);
    ASSERT_EQ(orderbook.getInsideAsk(), 9000);
    ASSERT_EQ(orderbook.getInsideAskSize(), 15);
    ASSERT_EQ(orderbook.getInsideBidSize(), 10);
}

