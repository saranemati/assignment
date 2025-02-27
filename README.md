[![Queue](https://github.com/saranemati/assignment/actions/workflows/queue.yml/badge.svg?branch=main)](https://github.com/saranemati/assignment/actions/workflows/queue.yml)
# CI assignment

Assignment Documentation:

queue.h

Description:

    - This header file defines the Queue class and the IMemory interface.
    
    - The Queue class is a circular queue that supports dynamic resizing at runtime.
    
    - The queue is movable but not copyable.
    
    - The IMemory interface provides memory allocation (malloc) and deallocation (free) methods.

Classes and Methods

IMemory (Interface)

    - virtual void* malloc(size_t size) = 0;Allocates memory of the given size and returns a pointer to it.
    
    - virtual void free(void* ptr) = 0;Deallocates memory pointed to by ptr.

Queue Class

Member Functions:

    - void clear(); - Clears the queue.
    
    - bool dequeue(T& item); - Removes the oldest element from the queue and stores it in item.
    
    - void enqueue(const T& item); - Adds a new item to the queue.
    
    - double average() ; - Returns the average value of all elements in the queue if the type is arithemtic.
    
    - void resize(size_t num); - Resizes the queue dynamically.
    
    - size_t size() ; - Returns the current number of elements in the queue.
    
    - bool isFull() ; - Checks if the number of elements is equal to the capacity of the queue.
    
    - size_t capacity() ; - Returns the maximum number of elements the queue can hold.

test.cpp

Description:

    - This file conatins unit tests for the Queue class.
    
    - Uses Google Test(gtest) and Google Mock(gmock) for testing.
    
    - Tests various queue functions including clear, dequeue, enqueue, the movable method, average and resize.

Testing Frameworks Used:

    - Google Test(gtest).
    
    - Google Mock(gmock).

Test Cases:

    - Initialization tests: Ensures that a queue is initialized properly and not with an invalid size.
    
    - Enqueue Tests: Checks if the function writes and overwrites correctly.
    
    - Dequeue Tests: Verifies that elements are removed in FIFO order.
    
    - Resize Tests: Ensures that rezising maintains data integrity.
    
    - Movability Tests: Tests the move constructor and move assignment operator.
    
    - Average Calculation Tests: Verifies that the average of queue elements is correctly counted and that it only works for arithemtic types.
    
