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

    Order&& order = orderbook.createOrder(QuoteType::BID, 10, 10, 100.4564);
    orderbook.addLimitOrder(std::make_shared<Order>(order), bid_compare);

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.getInsideBid(), 10046);
}

/* TEST(OrderBookTest, TestOrderBookTickSize) */
/* { */
/*     OrderBook orderbook{4}; */

/*     function<bool(double, double)> bid_compare; */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 100.4564, bid_compare); */

/*     std::cout << orderbook.getInsideBid(); */

/*     ASSERT_EQ(orderbook.size(), 1); */
/*     ASSERT_EQ(orderbook.getInsideBid(), 1004564); */
/* } */

/* TEST(OrderBookTest, TestOrderBookBidCreate) */
/* { */

/*     OrderBook orderbook; */

/*     function<bool(double, double)> bid_compare; */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */

/*     orderbook.addLimitOrder(QuoteType::BID, 1, 80, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::BID, 1, 90, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::BID, 1, 100, bid_compare); */

/*     ASSERT_TRUE(orderbook.size() == 3); */
/*     ASSERT_TRUE(orderbook.getInsideBid() == 10000); */
/* } */

/* TEST(OrderBookTest, TestOrderBookAskCreate) */
/* { */
/*     OrderBook orderbook; */

/*     function<bool(double, double)> ask_compare; */
/*     ask_compare = orderbook.buildCompareCallback(QuoteType::ASK); */

/*     orderbook.addLimitOrder(QuoteType::ASK, 1, 80, ask_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 1, 90, ask_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 1, 100, ask_compare); */

/*     ASSERT_TRUE(orderbook.size() == 3); */
/*     ASSERT_TRUE(orderbook.getInsideAsk() == 8000); */
/* } */

/* TEST(OrderBookTest, TestSingleBidAskExecute) */
/* { */
/*     OrderBook orderbook; */

/*     function<bool(double, double)> bid_compare; */
/*     function<bool(double, double)> ask_compare; */
/*     ask_compare = orderbook.buildCompareCallback(QuoteType::ASK); */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */

/*     // test single bid-ask execution */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 100, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 10, 100, ask_compare); */

/*     ASSERT_TRUE(orderbook.size() == 0); */
/*     ASSERT_TRUE(orderbook.getInsideBid() == 0); */
/*     ASSERT_TRUE(orderbook.getInsideAsk() == 0); */
/* } */

/* TEST(OrderBookTest, TestMultipleAskBidExecute) */
/* { */
/*     OrderBook orderbook; */

/*     function<bool(double, double)> bid_compare; */
/*     function<bool(double, double)> ask_compare; */
/*     ask_compare = orderbook.buildCompareCallback(QuoteType::ASK); */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */

/*     // test multiple ask to bid execution */
/*     orderbook.addLimitOrder(QuoteType::BID, 20, 100, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 10, 100, ask_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 10, 100, ask_compare); */

/*     ASSERT_TRUE(orderbook.size() == 0); */
/*     ASSERT_TRUE(orderbook.getInsideBid() == 0); */
/*     ASSERT_TRUE(orderbook.getInsideAsk() == 0); */
/* } */

/* TEST(OrderBookTest, TestMultipleBidAskExecute) */
/* { */
/*     OrderBook orderbook; */

/*     function<bool(double, double)> bid_compare; */
/*     function<bool(double, double)> ask_compare; */
/*     ask_compare = orderbook.buildCompareCallback(QuoteType::ASK); */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */

/*     // test multiple bid to ask execution */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 100, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 100, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 20, 100, ask_compare); */

/*     ASSERT_TRUE(orderbook.size() == 0); */
/*     ASSERT_TRUE(orderbook.getInsideBid() == 0); */
/*     ASSERT_TRUE(orderbook.getInsideAsk() == 0); */
/* } */

/* TEST(OrderBookTest, TestPartialAskExecution) */
/* { */
/*     OrderBook orderbook; */

/*     function<bool(double, double)> bid_compare; */
/*     function<bool(double, double)> ask_compare; */
/*     ask_compare = orderbook.buildCompareCallback(QuoteType::ASK); */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */

/*     // test partial ask execution */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 100, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 20, 100, ask_compare); */

/*     ASSERT_EQ(orderbook.size(), 1); */
/*     ASSERT_EQ(orderbook.getInsideBid(), 0); */
/*     ASSERT_EQ(orderbook.getInsideAsk(), 10000); */
/*     ASSERT_EQ(orderbook.getInsideAskSize(), 10); */
/* } */

/* TEST(OrderBookTest, TestPartialBidExecution) */
/* { */
/*     OrderBook orderbook; */

/*     function<bool(double, double)> bid_compare; */
/*     function<bool(double, double)> ask_compare; */
/*     ask_compare = orderbook.buildCompareCallback(QuoteType::ASK); */
/*     bid_compare = orderbook.buildCompareCallback(QuoteType::BID); */

/*     // test partial bid execution */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 80, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::BID, 10, 90, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::BID, 15, 90, bid_compare); */
/*     orderbook.addLimitOrder(QuoteType::ASK, 40, 90, ask_compare); */

/*     ASSERT_EQ(orderbook.size(), 2); */
/*     ASSERT_EQ(orderbook.getInsideBid(), 8000); */
/*     ASSERT_EQ(orderbook.getInsideAsk(), 9000); */
/*     ASSERT_EQ(orderbook.getInsideAskSize(), 15); */
/*     ASSERT_EQ(orderbook.getInsideBidSize(), 10); */
/* } */

