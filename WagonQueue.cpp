#include "WagonQueue.h"

const int WagonQueue::DEFAULT_CAPACITY;

WagonQueue::WagonQueue()
    : data(new int[DEFAULT_CAPACITY]), capacity(DEFAULT_CAPACITY), front(0), count(0)
{
    // Allocate an array of DEFAULT_CAPACITY wagons. The queue is empty, front = 0.
}

WagonQueue::~WagonQueue()
{
    // Free the array.
    delete[] data;
    data=nullptr;
}

WagonQueue::WagonQueue(const WagonQueue &other)
    : data(new int[other.capacity]), capacity(other.capacity), front(other.front), count(other.count)
{
    // Deep copy: an array of its own with other.capacity elements,
    //  the same wagons in the same order.
    for(int i=0; i<count; i++){
        int index=(front+i)%capacity;
        data[index]=other.data[index];
    }
}

WagonQueue &WagonQueue::operator=(const WagonQueue &other)
{
    // Deep copy. The old array must not leak, and q = q must change nothing.
    if(&other==this){
        return *this;
    }
    int *newData = new int[other.capacity];
    for (int i = 0; i < other.count; i++)
    {
        int index = (other.front + i) % other.capacity;
        newData[index] = other.data[index];
    }
    delete[] data;
    data = newData;
    capacity = other.capacity;
    front = other.front;
    count = other.count;
    return *this;
    
}

bool WagonQueue::isEmpty() const
{
    return count==0;
}

int WagonQueue::size() const
{
    return count;
}

void WagonQueue::resize(int newCapacity)
{
    // A new array of newCapacity elements. Copy the wagons IN QUEUE ORDER
    //  to indices 0 .. count-1 and set front = 0. Delete the old array.
    int *newArr=new int[newCapacity];
    for(int i=0; i<count; i++){
        newArr[i]=data[(front+i)%capacity];
    }
    delete[] data;
    data=newArr;
    front=0;
    capacity=newCapacity;
}

void WagonQueue::enqueue(int wagon)
{
    // If the array is full, resize(2 * capacity) first.
    // Then write the wagon at (front + count) % capacity.
    if(capacity==count){
        resize(2*capacity);
    }
    data[(front+count)%capacity]=wagon;
    count++;
}

bool WagonQueue::dequeue(int &out)
{
    // Return false if empty. Otherwise write the front wagon into out
    //  and move front one step (circularly).
    if(isEmpty()){
        return false;
    }
    out=data[front];
    front=(front+1)%capacity;
    count--;
    return true;
}

bool WagonQueue::peek(int &out) const
{
    // Return false if empty. Otherwise write the front wagon into out.
    if(isEmpty()){
        return false;
    }
    out=data[front];
    return true;
}

int WagonQueue::get(int i) const
{
    // The i-th wagon from the FRONT (0 = front).
    return data[(front+i)%capacity];
}

int WagonQueue::indexOf(int wagon) const
{
    // The position of the wagon from the FRONT, or -1 if it is not in the queue.
    for(int i=0; i<count; i++){
        int index=(front+i)%capacity;
        if(data[index]==wagon){
            return i;
        }
    }
    return -1;
}

void WagonQueue::clear()
{
    // count = 0 and front = 0. The capacity does not change.
    count=0;
    front=0;
}


