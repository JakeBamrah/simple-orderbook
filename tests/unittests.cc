#include <assert.h>
#include <functional>
#include <gtest/gtest.h>

#include "../src/orderbook.cc"

using std::function;

// TODO: write market order and cancel order tests

TEST(OrderTest, TestOrderInitialize)
{
    Order o{ 1, 1, QuoteType::BID, 100, 0, 100454 };

    ASSERT_EQ(o.id, 1);
    ASSERT_EQ(o.created_at, 1);
    ASSERT_EQ(o.quote_type, QuoteType::BID);
    ASSERT_EQ(o.open_quantity(), 100);
    ASSERT_EQ(o.filled_quantity, 0);
    ASSERT_EQ(o.price, 100454);
}

TEST(OrderTest, TestOrderFill)
{
    Order o{ 1, 1, QuoteType::BID, 100, 0, 100454 };
    o.fill(90, 90*90104, 1);

    ASSERT_EQ(o.open_quantity(), 10);
    ASSERT_EQ(o.filled_quantity, 90);
    ASSERT_EQ(o.filled_cost, 8109360);
}


TEST(LimitTest, TestLimitInitialize)
{
    Limit l1{100454};
    Limit l2{100468};
    l1.next = &l2;

    ASSERT_EQ(l1.price, 100454);
    ASSERT_EQ(l1.next, &l2);
    ASSERT_EQ(l1.total_volume, 0);
    ASSERT_EQ(l1.size, 0);
}

TEST(LimitTest, TestLimitAddOrder)
{
    Limit l1{100454};
    Order o{ 1, 1, QuoteType::BID, 100, 0, 100454 };
    l1.addOrder(std::make_shared<Order>(o));

    ASSERT_EQ(l1.size, 1);
}

TEST(LimitTest, TestLimitRemoveOrder)
{
    Limit l1{100454};
    Order o1{ 1, 1, QuoteType::BID, 100, 0, 100454 };
    Order o2{ 1, 1, QuoteType::BID, 100, 0, 100454 };
    Order o3{ 1, 1, QuoteType::BID, 100, 0, 100454 };

    shared_ptr<Order> o1p = std::make_shared<Order>(o1);
    shared_ptr<Order> o2p = std::make_shared<Order>(o2);
    shared_ptr<Order> o3p = std::make_shared<Order>(o3);
    l1.addOrder(o1p);
    l1.addOrder(o2p);
    l1.addOrder(o3p);

    l1.removeOrder(o2p);
    ASSERT_EQ(l1.size, 2);

    l1.removeOrder(o1p);
    ASSERT_EQ(l1.size, 1);

    l1.removeOrder(o3p);
    ASSERT_EQ(l1.size, 0);
}

TEST(OrderBookTest, TestOrderBookInitialize)
{
    OrderBook orderbook;
    ASSERT_EQ(orderbook.size(), 0);
    ASSERT_EQ(orderbook.inside_bid_price(), 0);
    ASSERT_EQ(orderbook.inside_ask_price(), 0);
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

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 0, 100.4564);
    orderbook.addLimitOrder(o1, bid_compare);

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.inside_bid_price(), 10046);
}

TEST(OrderBookTest, TestOrderBookTickSize)
{
    OrderBook orderbook{4};

    function<bool(double, double)> bid_compare;
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 0, 100.4564);
    orderbook.addLimitOrder(o1, bid_compare);

    std::cout << orderbook.inside_bid_price();

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.inside_bid_price(), 1004564);
}

TEST(OrderBookTest, TestOrderBookBidCreate)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 1, 0, 80);
    Order o2 = orderbook.createOrder(QuoteType::BID, 1, 0, 90);
    Order o3 = orderbook.createOrder(QuoteType::BID, 1, 0, 100);

    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);

    ASSERT_TRUE(orderbook.size() == 3);
    ASSERT_TRUE(orderbook.inside_bid_price() == 10000);
}

TEST(OrderBookTest, TestOrderBookAskCreate)
{
    OrderBook orderbook;

    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 1, 0, 80);
    Order o2 = orderbook.createOrder(QuoteType::ASK, 1, 0, 90);
    Order o3 = orderbook.createOrder(QuoteType::ASK, 1, 0, 100);

    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, ask_compare);
    orderbook.addLimitOrder(o3, ask_compare);

    ASSERT_TRUE(orderbook.size() == 3);
    ASSERT_TRUE(orderbook.inside_ask_price() == 8000);
}

TEST(OrderBookTest, TestSingleBidAskExecute)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 10, 0, 100);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 0, 100);

    // test single bid-ask execution
    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, bid_compare);

    ASSERT_TRUE(orderbook.size() == 0);
    ASSERT_TRUE(orderbook.inside_bid_price() == 0);
    ASSERT_TRUE(orderbook.inside_ask_price() == 0);
}

TEST(OrderBookTest, TestMultipleAskBidExecute)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 20, 0, 100);
    Order o2 = orderbook.createOrder(QuoteType::ASK, 10, 0, 100);
    Order o3 = orderbook.createOrder(QuoteType::ASK, 10, 0, 100);

    // test multiple ask to bid execution
    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, ask_compare);
    orderbook.addLimitOrder(o3, ask_compare);

    ASSERT_TRUE(orderbook.size() == 0);
    ASSERT_TRUE(orderbook.inside_bid_price() == 0);
    ASSERT_TRUE(orderbook.inside_ask_price() == 0);
}

TEST(OrderBookTest, TestMultipleBidAskExecute)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 20, 0, 100);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 0, 100);
    Order o3 = orderbook.createOrder(QuoteType::BID, 10, 0, 100);

    // test multiple bid to ask execution
    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);

    ASSERT_TRUE(orderbook.size() == 0);
    ASSERT_TRUE(orderbook.inside_bid_price() == 0);
    ASSERT_TRUE(orderbook.inside_ask_price() == 0);
}

TEST(OrderBookTest, TestPartialAskExecution)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::ASK, 20, 0, 100);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 0, 100);

    // test partial ask execution
    orderbook.addLimitOrder(o1, ask_compare);
    orderbook.addLimitOrder(o2, bid_compare);

    ASSERT_EQ(orderbook.size(), 1);
    ASSERT_EQ(orderbook.inside_bid_price(), 0);
    ASSERT_EQ(orderbook.inside_ask_price(), 10000);
    ASSERT_EQ(orderbook.inside_ask_quantity(), 10);
}

TEST(OrderBookTest, TestPartialBidExecution)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 0, 80);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 0, 90);
    Order o3 = orderbook.createOrder(QuoteType::BID, 15, 0, 90);
    Order o4 = orderbook.createOrder(QuoteType::ASK, 40, 0, 90);

    // test partial bid execution
    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);
    orderbook.addLimitOrder(o4, ask_compare);

    ASSERT_EQ(orderbook.size(), 2);
    ASSERT_EQ(orderbook.inside_bid_price(), 8000);
    ASSERT_EQ(orderbook.inside_ask_price(), 9000);
    ASSERT_EQ(orderbook.inside_ask_quantity(), 15);
    ASSERT_EQ(orderbook.inside_bid_quantity(), 10);
}

TEST(OrderBookTest, TestFilledOrderProperties)
{
    OrderBook orderbook;

    function<bool(double, double)> bid_compare;
    function<bool(double, double)> ask_compare;
    ask_compare = orderbook.buildCompareCallback(QuoteType::ASK);
    bid_compare = orderbook.buildCompareCallback(QuoteType::BID);

    Order o1 = orderbook.createOrder(QuoteType::BID, 10, 0, 80);
    Order o2 = orderbook.createOrder(QuoteType::BID, 10, 0, 90);
    Order o3 = orderbook.createOrder(QuoteType::BID, 15, 0, 90);
    Order o4 = orderbook.createOrder(QuoteType::ASK, 40, 0, 90);

    // test order fill calculations
    orderbook.addLimitOrder(o1, bid_compare);
    orderbook.addLimitOrder(o2, bid_compare);
    orderbook.addLimitOrder(o3, bid_compare);
    orderbook.addLimitOrder(o4, ask_compare);

    ASSERT_EQ(orderbook.size(), 2);
    ASSERT_EQ(o4.open_quantity(), 15);
    ASSERT_EQ(o4.filled_quantity, 25);
    ASSERT_EQ(o4.filled_cost, 225000);
}

