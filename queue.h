/**
 * @file queue.h
 * @author Sara Nemati (sara.nemati@yaelev.se)
 * @brief header file for the queue class, including the queue class and the IMemory class.
 *        functions in the queue class: clear, dequeue, enqueue, movable, average, resize (and also size, isFull and capacity).
 *        functions in the IMemory class: malloc, free.
 *        the queue class is a circular queue that can be resized at runtime.
 *        the queue class is movable but not copyable.
 *
 * @version 0.1
 * @date 2025-02-27
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include <cstddef>
#include <type_traits>
#include <new>
#include <vector>

struct IMemory // initailizing the IMemory class
{
    virtual void *malloc(size_t size) = 0;
    virtual void free(void *ptr) = 0;
    virtual ~IMemory() = default;
};

template <typename T> // initializing the queue class
class Queue
{

    struct node_t // initializing the node_t struct
    {
        T data;
        node_t *next{nullptr};
    };

    IMemory &memory;       // initializing the memory class
    size_t count{0};       // runtime-configurable
    size_t maxSize;        // runtime-configurable
    node_t *head{nullptr}; // initializing the head and tail
    node_t *tail{nullptr};

public:
    Queue(const Queue &) = delete; // deleting the copy constructor and the copy assignment operator
    Queue &operator=(const Queue &) = delete;
    // defining the constructor
    Queue(IMemory &_memory, size_t size) : memory{_memory}, maxSize{size}
    {
        if (maxSize < 3) // checking if the size is less than 3
        {
            throw std::invalid_argument("Size of queue less than 3."); // throw an exception for invalid size
        }

        for (size_t i = 0; i < maxSize; i++) // initializing the queue with the size
        {
            node_t *node = static_cast<node_t *>(memory.malloc(sizeof(node_t))); // allocating memory for the node

            if (node == nullptr) // checking if the memory allocation failed
            {
                // release memory for all nodes allocated so far
                while (head != nullptr)
                {
                    tail = head;
                    head = head->next;
                    tail->~node_t();
                    memory.free(tail);
                }

                throw std::bad_alloc(); // throw an exception for bad allocation
            }

            (void)new (node) node_t; // initializing the node

            if (head == nullptr) // checking if the head is null and initializing the head and tail
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

        tail->next = head; // linking the tail to the head creating a circular queue
    }

    Queue(Queue &&that) noexcept; // moving constructor

    Queue &operator=(Queue &&that) noexcept; // moving assignment operator

    void enqueue(const T &item); // initializing the enqueue function

    bool dequeue(T &item); // initializing the dequeue function

    bool isFull(); // initializing the isFull function

    double average(); // initializing the average function

    void resize(size_t num); // initializing the resize function

    size_t size(void) { return count; } // defining the size function

    size_t capacity(void) { return maxSize; } // defining the capacity function

    void clear(void); // initializing the clear function

    ~Queue() // defining the destructor
    {
        for (size_t i = 0; i < maxSize; i++) // removing and releasing the memory for all nodes
        {
            tail = head;
            head = head->next;
            tail->~node_t();
            memory.free(tail);
        }
    }
};

/**
 * @brief moving constructor
 * 
 *
 * @tparam T 
 * @param that 
 */
template <typename T> 
Queue<T>::Queue(Queue<T> &&that) noexcept : memory{that.memory}, count{that.count}, maxSize{that.maxSize}, head{that.head}, tail{that.tail}
{
    that.count = 0;
    that.maxSize = 0;
    that.head = nullptr;
    that.tail = nullptr;
}

/**
 * @brief moving assignment operator
 * 
 *
 * @tparam T 
 * @param that 
 * @return *this
 */
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

/**
 * @brief enqueue function to write an item to the queue and overwrite the oldest item if the queue is full
 *
 * @tparam T
 * @param item
 */
template <typename T>
void Queue<T>::enqueue(const T &item)
{
    tail = tail->next; // moving the tail to the next node
    tail->data = item; // adding the item to the tail

    if (!isFull()) // if queue is not full increment the count
    {
        count++;
    }
    else // if queue is full overwrite and move the head to the next node
    {
        head = head->next;
    }
}

/**
 * @brief dequeue function to read an item from the queue
 *
 *
 * @tparam T
 * @param item
 * @return true
 * @return false
 */
template <typename T>
bool Queue<T>::dequeue(T &item)
{
    bool status{false};

    if (count > 0)
    {
        item = head->data; // reading the item from the head
        head = head->next; // moving the head to the next node
        count--;           // decrementing the count
        status = true;     // setting the status to true
    }

    return status;
}

/**
 * @brief is full function to check if the queue is full
 *
 * @tparam T
 * @return true
 * @return false
 */
template <typename T>
bool Queue<T>::isFull()
{
    return count == maxSize; // checking if the count is equal to the capacity of the queue
}

/**
 * @brief average function to calculate the average of the elements in the queue if the type is arithmetic
 *
 *
 * @tparam T
 * @return double
 */
template <typename T>
double Queue<T>::average()
{
    double value{0};                                                                          // initializing the value
    static_assert(std::is_arithmetic<T>::value, "average() only works for arithmetic types"); // checking if the type is arithmetic

    if (count > 0) // if the count is greater than 0 calculate the average
    {
        node_t *current = head;            // initializing the current node
        for (size_t i = 0; i < count; i++) // calculating the average
        {
            value += static_cast<double>(current->data); // adding the value of the current node to the value
            current = current->next;                     // moving to the next node
        }

        value = value / count; // calculating the average
    }

    return value; // returning the average
}

/**
 * @brief resize function to resize the queue at runtime, increase or decrease size, adding or removing nodes accordingly
 *
 *
 * @tparam T
 * @param num
 */
template <typename T>
void Queue<T>::resize(size_t num)
{
    if (num < 3) // checking if the size is less than 3
    {
        throw std::invalid_argument("size too small."); // throw an exception for invalid size
    }

    if (num > maxSize) // for increasing the size
    {
        for (size_t i = maxSize; i < num; i++) // adding nodes to the queue starting from current size to the new size
        {
            node_t *node = reinterpret_cast<node_t *>(memory.malloc(sizeof(node_t))); // allocating memory for the node

            if (node == nullptr) // checking if the memory allocation failed
            {
                for (size_t j = maxSize; j < i; j++) // releasing memory for all nodes allocated so far
                {
                    node = tail->next;
                    tail->next = node->next;
                    node->~node_t();
                    memory.free(node);
                }

                throw std::bad_alloc(); // throw an exception for bad allocation
            }
            else // if memory allocation is successful
            {
                (void)new (node) node_t; // initializing the node
                node->next = tail->next; // inserting the node between tail->next and tail
                tail->next = node;
            }
        }
    }
    else if (num < maxSize) // for decreasing the size
    {
        node_t *node{nullptr};                 // initializing the node used to delete nodes
        for (size_t i = num; i < maxSize; i++) // removing nodes from the queue starting from new size to current size
        {
            node = tail->next; // deleting the node after the tail
            if (node == head)  // if the node is the head move the head to the next node
            {
                head = head->next;
                count--; // decrementing the count
            }
            tail->next = node->next; // setting the tail->next to the next node after the node that shall be deleted

            node->~node_t();
            memory.free(node);
        }
    }

    maxSize = num; // setting the new size accordingly
}

/**
 * @brief clear function to reset the queue to its initial state, by resetting the count and moving the head to the next node
 *
 *
 * @tparam T
 * @param
 */
template <typename T>
void Queue<T>::clear(void)
{
    count = 0;
    head = tail->next;
}
#endif // QUEUE_H