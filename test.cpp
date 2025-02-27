/**
 * @file test.cpp
 * @author Sara Nemati (sara.nemati@yaelev.se)
 * @brief test file for the queue class, including tests for the functions in the queue class: 
 *                  clear, dequeue, enqueue, movable, average, resize.
 * @version 0.1
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "queue.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
// initializing the memory class
class Memory : public IMemory
{
    size_t pos{0};
    uint8_t array[8192];
    std::vector<void *> vec;

public:
    MOCK_METHOD(void *, malloc, (size_t size), (override)); // mocking the malloc function
    MOCK_METHOD(void, free, (void *ptr), (override));       // mocking the free function

    void *allocate(size_t size) // allocating memory
    {
        void *ptr{nullptr};

        if ((pos + size) < sizeof(array))
        {
            ptr = &array[pos];
            vec.push_back(ptr);
            pos += size;
        }

        return ptr;
    }

    void release(void *ptr) // releasing memory
    {
        vec.erase(std::remove(vec.begin(), vec.end(), ptr), vec.end());
    }

    ~Memory() { EXPECT_EQ(0, vec.size()); } // destructor
};
// initializing the queue class for testing
template <typename T>
class QueueFixture : public ::testing::Test
{ // initializing the values
    const std::tuple<
        std::vector<int>,
        std::vector<float>,
        std::vector<std::string>>
        allValues{
            {1, 2, 3, 4, 5},
            {1.5f, 2.5f, 3.5f, 4.5f, 5.5f},
            {"aaa", "bbb", "ccc", "ddd", "eee"}};

protected:
    const std::vector<T> values{std::get<std::vector<T>>(allValues)}; // getting the values
    NiceMock<Memory> mock;                                            // mocking the memory class
    Queue<T> *queue{nullptr};                                         // initializing the queue

    void SetUp(void) override // setting up the queue
    {
        ON_CALL(mock, malloc(_)) // mocking the malloc function
            .WillByDefault(Invoke(&mock, &Memory::allocate));

        ON_CALL(mock, free(_)) // mocking the free function
            .WillByDefault(Invoke(&mock, &Memory::release));

        queue = new Queue<T>{mock, values.size()}; // creating the queue

        for (size_t i = 1; i <= values.size(); i++) // enqueuing the values
        {
            queue->enqueue(values[i - 1]);
            EXPECT_EQ(i, queue->size());
        }
    }

    void TearDown(void) override // tearing down the queue
    {
        delete queue;
    }
};
// initializing the test types
using TestTypes = ::testing::Types<int, float, std::string>;
TYPED_TEST_SUITE(QueueFixture, TestTypes); // creating the test suite

TYPED_TEST(QueueFixture, InvalidSize) // testing the invalid size
{
    EXPECT_THROW(Queue<TypeParam>(this->mock, 2), std::invalid_argument);
}

TYPED_TEST(QueueFixture, mallocFails) // testing the malloc function
{
    EXPECT_CALL(this->mock, malloc(_)) // calling the malloc function
        .WillOnce(Return(nullptr));
    EXPECT_THROW(Queue<TypeParam>(this->mock, this->values.size()), std::bad_alloc); // throw expected for bad allocation

    EXPECT_CALL(this->mock, malloc(_))                                               // calling the malloc function
        .WillOnce(Invoke(&this->mock, &Memory::allocate))                            // allocation successful
        .WillOnce(Invoke(&this->mock, &Memory::allocate))                            // allocation successful
        .WillOnce(Return(nullptr));                                                  // allocation failed
    EXPECT_THROW(Queue<TypeParam>(this->mock, this->values.size()), std::bad_alloc); // throw expected for bad allocation
}

TYPED_TEST(QueueFixture, testClear) // testing the clear function
{
    TypeParam item;                                      // initializing the item
    EXPECT_EQ(this->values.size(), this->queue->size()); // checking the size
    this->queue->clear();                                // calling the clear function to clear the queue
    EXPECT_EQ(0, this->queue->size());                   // checking the size
    EXPECT_FALSE(this->queue->dequeue(item));            // expecting false when dequeuing since there is no elements
}

TYPED_TEST(QueueFixture, testDequeue) // testing the dequeue function
{
    TypeParam item;                   // initializing the item
    size_t size{this->queue->size()}; // getting the size

    for (size_t i = 1; i <= size; i++) // dequeuing the elements
    {
        EXPECT_TRUE(this->queue->dequeue(item));
        EXPECT_EQ(size - i, this->queue->size());
        EXPECT_EQ(item, this->values[i - 1]);
    }

    EXPECT_FALSE(this->queue->dequeue(item)); // expecting false when dequeuing since there is no elements left
}

TYPED_TEST(QueueFixture, testEnqueueFull) // testing the enqueue function when the queue is full
{
    TypeParam item;                     // initializing the item
    EXPECT_TRUE(this->queue->isFull()); // expecting true since the queue is full at initialization

    // Overwrite oldest element
    this->queue->enqueue(this->values[0]);
    EXPECT_TRUE(this->queue->isFull());
    EXPECT_EQ(this->values.size(), this->queue->size());

    // Dequeue should return the second oldest element (not the overwritten one)
    EXPECT_TRUE(this->queue->dequeue(item));
    EXPECT_EQ(item, this->values[1]);
}

TYPED_TEST(QueueFixture, testOnlyMovable) // testing if the queue is movable
{
    EXPECT_FALSE(std::is_copy_constructible<Queue<TypeParam>>::value); // checking if the queue is copy constructible
    EXPECT_FALSE(std::is_copy_assignable<Queue<TypeParam>>::value);    // checking if the queue is copy assignable

    EXPECT_TRUE(std::is_move_constructible<Queue<TypeParam>>::value);  // checking if the queue is move constructible
    EXPECT_TRUE(std::is_move_assignable<Queue<TypeParam>>::value);     // checking if the queue is move assignable

    Queue<TypeParam> temp{std::move(*this->queue)};                    // moving the queue to a temporary queue
    EXPECT_EQ(this->values.size(), temp.size());                       // the size of the temporary queue should be equal to the values
    EXPECT_EQ(0, this->queue->size());                                 // the size of the original queue should be 0

    *this->queue = std::move(temp);                                    // moving the temporary queue back to the original queue
    EXPECT_EQ(this->values.size(), this->queue->size());               // the size of the original queue should be equal to the values
    EXPECT_EQ(0, temp.size());                                         // the size of the temporary queue should be 0
}

TYPED_TEST(QueueFixture, testAverage) // testing the average function
{                                     // checking if the type is arithmetic
    if constexpr (std::is_arithmetic<TypeParam>::value)
    {
        double sum = 0;                      // initializing the sum
        for (const auto &val : this->values) // calculating the sum
        {
            sum += val;
        }

        double expectedAvg = sum / this->values.size();        // calculating the expected average
        EXPECT_DOUBLE_EQ(this->queue->average(), expectedAvg); // checking if the average is equal to the expected average
    }
}

TYPED_TEST(QueueFixture, testResize) // testing the resize function
{                                    // getting the size and testing the resize function with an invalid size
    size_t size = this->values.size();
    EXPECT_THROW(this->queue->resize(2), std::invalid_argument);

    size += 4; // increaing variable size and resizing the queue
    EXPECT_NO_THROW(this->queue->resize(size));
    EXPECT_FALSE(this->queue->isFull());
    EXPECT_EQ(this->values.size(), this->queue->size());
    EXPECT_EQ(size, this->queue->capacity()); // size should be equal to the capacity

    EXPECT_CALL(this->mock, malloc(_))                           // calling the malloc function
        .WillOnce(Return(nullptr));                              // allocation failed
    EXPECT_THROW(this->queue->resize(size + 3), std::bad_alloc); // throw expected for bad allocation
    EXPECT_EQ(this->values.size(), this->queue->size());         // size of the queue should be equal to the values
    EXPECT_EQ(size, this->queue->capacity());                    // the capacity should be equal to variable size

    EXPECT_CALL(this->mock, malloc(_))                           // calling the malloc function
        .WillOnce(Invoke(&this->mock, &Memory::allocate))        // allocation successful
        .WillOnce(Invoke(&this->mock, &Memory::allocate))        // allocation successful
        .WillOnce(Return(nullptr));                              // allocation failed
    EXPECT_THROW(this->queue->resize(size + 4), std::bad_alloc); // throw expected for bad allocation
    EXPECT_EQ(this->values.size(), this->queue->size());         // size of the queue should be equal to the values
    EXPECT_EQ(size, this->queue->capacity());                    // the capacity should be equal to variable size
}