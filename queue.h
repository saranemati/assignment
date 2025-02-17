#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <type_traits>

struct IMemory
{
    virtual void *malloc(size_t size) = 0;
    virtual void free(void *ptr) = 0;
    virtual ~IMemory() = default;
};

template <typename T, size_t N>
class Queue
{
    static_assert(N > 2, "Queue must be at least 3!");

    struct node_t
    {
        T data;
        node_t *next;
    };

    IMemory &memory;
    size_t count{0};
    node_t *head{nullptr};
    node_t *tail{nullptr};

public:
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    Queue(IMemory &_memory) : memory{_memory} {}

    Queue(Queue &&that) noexcept;

    Queue &operator=(Queue &&that) noexcept;

    bool enqueue(const T &item);

    bool dequeue(T &item);

    bool isFull();

    double average();

    size_t size(void) { return count; }

    void clear(void);

    ~Queue() { clear(); }
};

template <typename T, size_t N>
Queue<T, N>::Queue(Queue<T, N> &&that) noexcept : memory{that.memory}, count{that.count}, head{that.head}, tail{that.tail}
{
    that.count = 0;
    that.head = nullptr;
    that.tail = nullptr;
}

template <typename T, size_t N>
Queue<T, N> &Queue<T, N>::operator=(Queue<T, N> &&that) noexcept
{
    if (this != &that)
    {
        clear();

        memory = that.memory;
        count = that.count;
        head = that.head;
        tail = that.tail;

        that.count = 0;
        that.head = nullptr;
        that.tail = nullptr;
    }

    return *this;
}

template <typename T, size_t N>
bool Queue<T, N>::enqueue(const T &item)
{
    bool status{false};

    node_t *node{static_cast<node_t *>(memory.malloc(sizeof(node_t)))};

    if (node != nullptr)
    {
        (void)new (node) node_t{item, nullptr};

        if (head == nullptr)
        {
            head = node;
            tail = head;
            tail->next = head;
        }
        else
        {
            tail->next = node;
            tail = node;
            tail->next = head;
        }

        status = true;
        count++;
    }

    return status;
}

template <typename T, size_t N>
bool Queue<T, N>::dequeue(T &item)
{
    bool status{false};

    if (count <= N) // Prevent shrinking below MinSize
    {
        return false;
    }

    if (head != nullptr)
    {
        item = head->data;

        node_t *temp{head};
        head = head->next;
        temp->~node_t();
        memory.free(temp);

        if (head == nullptr)
        {
            tail = head;
        }

        status = true;
        count--;
    }

    return status;
}

template <typename T, size_t N>
bool Queue<T, N>::isFull()
{
    return count == N;
}

template <typename T, size_t N>
double Queue<T, N>::average()
{
    if (isalpha(T))
    {
        double sum = 0;
        node_t *current = head;

        do
        {
            sum += current->data;
            current = current->next;
        } while (current != head);
    }
    return sum / count;
}

template <typename T, size_t N>
void Queue<T, N>::clear(void)
{
    while (head != nullptr)
    {
        tail = head;
        head = head->next;
        tail->~node_t();
        memory.free(tail);
    }

    count = 0;
    tail = head;
}
#endif // QUEUE_H