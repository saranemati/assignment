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

template <typename T>
class Queue
{

    struct node_t
    {
        T data;
        node_t *next;
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
            maxSize = 3; // Ensure at least 3 elements

        node_t *prev = nullptr;
        for (size_t i = 0; i < maxSize; i++)
        {
            node_t *node = static_cast<node_t *>(memory.malloc(sizeof(node_t)));
            if (!node)
                break;
            (void)new (node) node_t{T{}, nullptr};
            if (!head)
                head = node;
            else
                prev->next = node;
            prev = node;
        }
        tail = prev;
        if (tail)
            tail->next = head; // Make circular
    }

    Queue(Queue &&that) noexcept;

    Queue &operator=(Queue &&that) noexcept;

    bool enqueue(const T &item);

    bool dequeue(T &item);

    bool isFull();

    double average();

    bool resize(size_t num);

    size_t size(void) { return count; }

    void clear(void);

    ~Queue() { clear(); }
};

template <typename T>
Queue<T>::Queue(Queue<T> &&that) noexcept : memory{that.memory}, count{that.count}, head{that.head}, tail{that.tail}
{
    that.count = 0;
    that.head = nullptr;
    that.tail = nullptr;
}

template <typename T>
Queue<T> &Queue<T>::operator=(Queue<T> &&that) noexcept
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

template <typename T>
bool Queue<T>::enqueue(const T &item)
{
    bool status{false};

    if (isFull())
    {
        dequeue(head->data); // Overwrite oldest if full
    }

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
            node->next = head;
            tail->next = node;
            tail = node;
        }

        status = true;
        count++;
    }

    return status;
}

template <typename T>
bool Queue<T>::dequeue(T &item)
{
    bool status{false};

    if (count == 0) // Prevent shrinking below MinSize
    {
        return false;
    }

    if (head != nullptr)
    {
        item = head->data; // Extract the item
        node_t *temp = head;

        if (head == tail) // Only one element in the queue
        {
            head = nullptr;
            tail = nullptr;
        }
        else
        {
            head = head->next; // Move head forward
            tail->next = head; // Maintain circular linking
        }
        memory.free(temp);

        status = true;
        count--;
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
    static_assert(std::is_arithmetic<T>::value, "average() only works for arithmetic types");

    double sum = 0;
    node_t *current = head;
    size_t elements = 0;

    do
    {
        sum += current->data;
        current = current->next;
        elements++;
    } while (current != head && elements < count);

    return sum / elements;
}

template <typename T>
bool Queue<T>::resize(size_t num)
{
    // enter new size (at least bigger than 2)
    // if new size is smaller remove the oldest nodes first
    // set new size
    bool status{false};
    if (num < 3)
    {
        status = false;
    }
    else
    {
        while (count > num)
        {
            T temp;
            dequeue(temp);
        }
        maxSize = num;
        status = true;
    }
    return status;
}

template <typename T>
void Queue<T>::clear(void)
{
    while (count > 0)
    {
        T temp;
        dequeue(temp);
    }
}
#endif // QUEUE_H