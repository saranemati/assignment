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

    size_t memory_blocks(void) { return vec.size(); }

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

template <typename T, size_t N>
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
    const std::vector<T, N> values{std::get<std::vector<T>>(allValues)};
    NiceMock<Memory> mock;
    Queue<T, N> queue{mock};

    void SetUp(void) override
    {
        EXPECT_EQ(0, queue.size());

        ON_CALL(mock, malloc(_))
            .WillByDefault(Invoke(&mock, &Memory::allocate));

        ON_CALL(mock, free(_))
            .WillByDefault(Invoke(&mock, &Memory::release));

        for (size_t i = 1; i <= values.size(); i++)
        {
            EXPECT_TRUE(queue.enqueue(values[i - 1]));
            EXPECT_EQ(i, queue.size());
        }
    }

    void TearDown(void) override {}
};

using TestTypes = ::testing::Types<int, float, std::string>;
TYPED_TEST_SUITE(QueueFixture, TestTypes);

TYPED_TEST(QueueFixture, mallocFails)
{
    EXPECT_CALL(this->mock, malloc(_))
        .WillRepeatedly(Return(nullptr));
    EXPECT_FALSE(this->queue.enqueue(this->values[0]));
}
TYPED_TEST(QueueFixture, testClear)
{
    EXPECT_EQ(this->values.size(), this->queue.size());
    this->queue.clear();
    EXPECT_EQ(0, this->queue.size());
    EXPECT_EQ(0, this->mock.memory_blocks());
}

TYPED_TEST(QueueFixture, testDequeue)
{
    TypeParam item;
    size_t size{this->queue.size()};

    for (size_t i = 1; i <= size; i++)
    {
        EXPECT_TRUE(this->queue.dequeue(item));
        EXPECT_EQ(size - i, this->queue.size());
        EXPECT_EQ(item, this->values[i - 1]);
    }

    EXPECT_FALSE(this->queue.dequeue(item));
}

TYPED_TEST(QueueFixture, testOnlyMovable)
{
    EXPECT_FALSE(std::is_copy_constructible<Queue<TypeParam>>::value);
    EXPECT_FALSE(std::is_copy_assignable<Queue<TypeParam>>::value);

    EXPECT_TRUE(std::is_move_constructible<Queue<TypeParam>>::value);
    EXPECT_TRUE(std::is_move_assignable<Queue<TypeParam>>::value);

    Queue<TypeParam> temp{std::move(this->queue)};
    EXPECT_EQ(this->values.size(), temp.size());
    EXPECT_EQ(0, this->queue.size());

    this->queue = std::move(temp);
    EXPECT_EQ(this->values.size(), this->queue.size());
    EXPECT_EQ(0, temp.size());
}