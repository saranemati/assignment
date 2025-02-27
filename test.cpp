#include "queue.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

class Memory : public IMemory
{
    size_t pos{0};
    uint8_t array[8192]; // Increase size of array if required
    std::vector<void *> vec;

public:
    MOCK_METHOD(void *, malloc, (size_t size), (override));
    MOCK_METHOD(void, free, (void *ptr), (override));

    void *allocate(size_t size)
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

    void release(void *ptr)
    {
        vec.erase(std::remove(vec.begin(), vec.end(), ptr), vec.end());
    }

    ~Memory() { EXPECT_EQ(0, vec.size()); }
};

template <typename T>
class QueueFixture : public ::testing::Test
{
    const std::tuple<
        std::vector<int>,
        std::vector<float>,
        std::vector<std::string>>
        allValues{
            {1, 2, 3, 4, 5},
            {1.5f, 2.5f, 3.5f, 4.5f, 5.5f},
            {"aaa", "bbb", "ccc", "ddd", "eee"}};

protected:
    const std::vector<T> values{std::get<std::vector<T>>(allValues)};
    NiceMock<Memory> mock;
    Queue<T> *queue{nullptr};

    void SetUp(void) override
    {
        ON_CALL(mock, malloc(_))
            .WillByDefault(Invoke(&mock, &Memory::allocate));

        ON_CALL(mock, free(_))
            .WillByDefault(Invoke(&mock, &Memory::release));

        queue = new Queue<T>{mock, values.size()};

        for (size_t i = 1; i <= values.size(); i++)
        {
            queue->enqueue(values[i - 1]);
            EXPECT_EQ(i, queue->size());
        }
    }

    void TearDown(void) override
    {
        delete queue;
    }
};

using TestTypes = ::testing::Types<int, float, std::string>;
TYPED_TEST_SUITE(QueueFixture, TestTypes);

TYPED_TEST(QueueFixture, InvalidSize)
{
    EXPECT_THROW(Queue<TypeParam>(this->mock, 2), std::invalid_argument);
}

TYPED_TEST(QueueFixture, mallocFails)
{
    EXPECT_CALL(this->mock, malloc(_))
        .WillOnce(Return(nullptr));
    EXPECT_THROW(Queue<TypeParam>(this->mock, this->values.size()), std::bad_alloc);

    EXPECT_CALL(this->mock, malloc(_))
        .WillOnce(Invoke(&this->mock, &Memory::allocate))
        .WillOnce(Invoke(&this->mock, &Memory::allocate))
        .WillOnce(Return(nullptr));
    EXPECT_THROW(Queue<TypeParam>(this->mock, this->values.size()), std::bad_alloc);
}

TYPED_TEST(QueueFixture, testClear)
{
    TypeParam item;
    EXPECT_EQ(this->values.size(), this->queue->size());
    this->queue->clear();
    EXPECT_EQ(0, this->queue->size());
    EXPECT_FALSE(this->queue->dequeue(item));
}

TYPED_TEST(QueueFixture, testDequeue)
{
    TypeParam item;
    size_t size{this->queue->size()};

    for (size_t i = 1; i <= size; i++)
    {
        EXPECT_TRUE(this->queue->dequeue(item));
        EXPECT_EQ(size - i, this->queue->size());
        EXPECT_EQ(item, this->values[i - 1]);
    }

    EXPECT_FALSE(this->queue->dequeue(item));
}

TYPED_TEST(QueueFixture, testEnqueueFull)
{
    TypeParam item;
    EXPECT_TRUE(this->queue->isFull());

    // Overwrite oldest element
    this->queue->enqueue(this->values[0]);
    EXPECT_TRUE(this->queue->isFull());
    EXPECT_EQ(this->values.size(), this->queue->size());

    // Dequeue should return the second oldest element (not the overwritten one)
    EXPECT_TRUE(this->queue->dequeue(item));
    EXPECT_EQ(item, this->values[1]);
}

TYPED_TEST(QueueFixture, testOnlyMovable)
{
    EXPECT_FALSE(std::is_copy_constructible<Queue<TypeParam>>::value);
    EXPECT_FALSE(std::is_copy_assignable<Queue<TypeParam>>::value);

    EXPECT_TRUE(std::is_move_constructible<Queue<TypeParam>>::value);
    EXPECT_TRUE(std::is_move_assignable<Queue<TypeParam>>::value);

    Queue<TypeParam> temp{std::move(*this->queue)};
    EXPECT_EQ(this->values.size(), temp.size());
    EXPECT_EQ(0, this->queue->size());

    *this->queue = std::move(temp);
    EXPECT_EQ(this->values.size(), this->queue->size());
    EXPECT_EQ(0, temp.size());
}

TYPED_TEST(QueueFixture, testAverage)
{
    if constexpr (std::is_arithmetic<TypeParam>::value)
    {
        double sum = 0;
        for (const auto &val : this->values)
        {
            sum += val;
        }

        double expectedAvg = sum / this->values.size();
        EXPECT_DOUBLE_EQ(this->queue->average(), expectedAvg);
    }
}

TYPED_TEST(QueueFixture, testResize)
{
    size_t size = this->values.size();
    EXPECT_THROW(this->queue->resize(2), std::invalid_argument);

    size += 4;
    EXPECT_NO_THROW(this->queue->resize(size));
    EXPECT_FALSE(this->queue->isFull());
    EXPECT_EQ(this->values.size(), this->queue->size());
    EXPECT_EQ(size, this->queue->capacity());

    EXPECT_CALL(this->mock, malloc(_))
        .WillOnce(Return(nullptr));
    EXPECT_THROW(this->queue->resize(size + 3), std::bad_alloc);
    EXPECT_EQ(this->values.size(), this->queue->size());
    EXPECT_EQ(size, this->queue->capacity());

    EXPECT_CALL(this->mock, malloc(_))
        .WillOnce(Invoke(&this->mock, &Memory::allocate))
        .WillOnce(Invoke(&this->mock, &Memory::allocate))
        .WillOnce(Return(nullptr));
    EXPECT_THROW(this->queue->resize(size + 4), std::bad_alloc);
    EXPECT_EQ(this->values.size(), this->queue->size());
    EXPECT_EQ(size, this->queue->capacity());
}