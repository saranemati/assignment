#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include <cstddef>
#include <type_traits>
#include <new>
#include <vector>

struct IMemory
{
    virtual void *malloc(size_t size) = 0;
    virtual void free(void *ptr) = 0;
    virtual ~IMemory() = default;
};

template <typename T>
class Queue
{

    struct node_t
    {
        T data;
        node_t *next{nullptr};
    };

    IMemory &memory;
    size_t count{0};
    size_t maxSize; // Now runtime-configurable
    node_t *head{nullptr};
    node_t *tail{nullptr};

public:
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    Queue(IMemory &_memory, size_t size) : memory{_memory}, maxSize{size}
    {
        if (maxSize < 3)
        {
            throw std::invalid_argument("Size of queue less than 3.");
        }

        for (size_t i = 0; i < maxSize; i++)
        {
            node_t *node = static_cast<node_t *>(memory.malloc(sizeof(node_t)));

            if (node == nullptr)
            {
                // release memory
                while (head != nullptr)
                {
                    tail = head;
                    head = head->next;
                    tail->~node_t();
                    memory.free(tail);
                }

                throw std::bad_alloc();
            }

            (void)new (node) node_t;

            if (head == nullptr)
            {
                head = node;
                tail = head;
            }
            else
            {
                tail->next = node;
                tail = node;
            }
        }

        tail->next = head;
    }

    Queue(Queue &&that) noexcept;

    Queue &operator=(Queue &&that) noexcept;

    void enqueue(const T &item);

    bool dequeue(T &item);

    bool isFull();

    double average();

    void resize(size_t num);

    size_t size(void) { return count; }

    size_t capacity(void) { return maxSize; }

    void clear(void);

    ~Queue()
    {
        for (size_t i = 0; i < maxSize; i++)
        {
            tail = head;
            head = head->next;
            tail->~node_t();
            memory.free(tail);
        }
    }
};

template <typename T>
Queue<T>::Queue(Queue<T> &&that) noexcept : memory{that.memory}, count{that.count}, maxSize{that.maxSize}, head{that.head}, tail{that.tail}
{
    that.count = 0;
    that.maxSize = 0;
    that.head = nullptr;
    that.tail = nullptr;
}

template <typename T>
Queue<T> &Queue<T>::operator=(Queue<T> &&that) noexcept
{
    if (this != &that)
    {
        this->~Queue();

        memory = that.memory;
        count = that.count;
        maxSize = that.maxSize;
        head = that.head;
        tail = that.tail;

        that.count = 0;
        that.maxSize = 0;
        that.head = nullptr;
        that.tail = nullptr;
    }

    return *this;
}

template <typename T>
void Queue<T>::enqueue(const T &item)
{
    tail = tail->next;
    tail->data = item;

    if (!isFull())
    {
        count++;
    }
    else
    {
        head = head->next;
    }
}

template <typename T>
bool Queue<T>::dequeue(T &item)
{
    bool status{false};

    if (count > 0)
    {
        item = head->data;
        head = head->next;
        count--;
        status = true;
    }

    return status;
}

template <typename T>
bool Queue<T>::isFull()
{
    return count == maxSize;
}

template <typename T>
double Queue<T>::average()
{
    double value{0};
    static_assert(std::is_arithmetic<T>::value, "average() only works for arithmetic types");

    if (count > 0)
    {
        node_t *current = head;
        for (size_t i = 0; i < count; i++)
        {
            value += static_cast<double>(current->data);
            current = current->next;
        }

        value = value / count;
    }

    return value;
}

template <typename T>
void Queue<T>::resize(size_t num)
{
    if (num < 3)
    {
        throw std::invalid_argument("size too small.");
    }

    if (num > maxSize)
    {
        for (size_t i = maxSize; i < num; i++)
        {
            node_t *node = reinterpret_cast<node_t *>(memory.malloc(sizeof(node_t)));

            if (node == nullptr)
            {
                for (size_t j = maxSize; j < i; j++)
                {
                    node = tail->next;
                    tail->next = node->next;
                    node->~node_t();
                    memory.free(node);
                }

                throw std::bad_alloc();
            }
            else
            {
                (void)new (node) node_t;
                node->next = tail->next;
                tail->next = node;
            }
        }
    }
    else if (num < maxSize)
    {
        node_t *node{nullptr};
        for (size_t i = num; i < maxSize; i++)
        {
            node = tail->next;
            if (node == head)
            {
                head = head->next;
                count--;
            }
            tail->next = node->next;

            node->~node_t();
            memory.free(node);
        }
    }

    maxSize = num;
}

template <typename T>
void Queue<T>::clear(void)
{
    count = 0;
    head = tail->next;
}
#endif // QUEUE_H