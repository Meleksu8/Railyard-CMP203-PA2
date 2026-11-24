#include "WagonStack.h"

const int WagonStack::DEFAULT_CAPACITY;

WagonStack::WagonStack()
    : data(new int[DEFAULT_CAPACITY]), capacity(DEFAULT_CAPACITY), top(-1)
{
    // Allocate an array of DEFAULT_CAPACITY wagons. The stack is empty.
}

WagonStack::~WagonStack()
{
    // Free the array.
    delete[] data;
    data=nullptr;
}

WagonStack::WagonStack(const WagonStack &other)
    : data(new int[other.capacity]), capacity(other.capacity), top(other.top)
{
    // Deep copy: an array of its own with other.capacity elements and the same wagons.
    for(int i=0; i<=top; i++){
        data[i]=other.data[i];
    }
    
}

WagonStack &WagonStack::operator=(const WagonStack &other)
{
    // Deep copy. The old array must not leak, and s = s must change nothing.
    if (&other == this)
    {
        return *this;
    }
    int *newData = new int[other.capacity];
    for (int i = 0; i <= other.top; i++)
    {
        newData[i] = other.data[i];
    }
    delete[] data;
    data = newData;
    capacity = other.capacity;
    top = other.top;
    return *this;
    
}

bool WagonStack::isEmpty() const
{
    if(top==-1){
        return true;
    }else{
        return false;
    }
    
}

int WagonStack::size() const
{
    return top+1;
}

void WagonStack::resize(int newCapacity)
{
    // A new array of newCapacity elements with the same wagons at the same indices.
    // Delete the old array.
    int *newArr=new int[newCapacity];
    // to avoid copying unnecessary values, we copy only the actual values—not the empty spaces
    int size=top+1;
    for(int i=0; i<newCapacity && i<size ; i++){
        newArr[i]=data[i];
    }
    delete[] data;
    data=newArr;
    capacity=newCapacity;

}

void WagonStack::push(int wagon)
{
    // If the array is full, resize(2 * capacity) first. Then put the wagon on top.
    if (top+1==capacity)
    {
        resize(2*capacity);
    }
    top++;
    data[top]=wagon;
    
}

bool WagonStack::pop(int &out)
{
    // Return false if empty. Otherwise write the top wagon into out and remove it.
    if(isEmpty()){
        return false;
    }else{
        out=data[top];
        top--;
        return true;
    }
    
}

bool WagonStack::peek(int &out) const
{
    // Return false if empty. Otherwise write the top wagon into out.
    if(isEmpty()){
        return false;
    }else{
        out=data[top];
        return true;
    }
    
}

int WagonStack::get(int i) const
{
    // The i-th wagon from the BOTTOM (0 = bottom).
    return data[i];
    
}

int WagonStack::indexOf(int wagon) const
{
    // The position of the wagon from the BOTTOM, or -1 if it is not in the stack.
    for(int i=0; i<=top; i++){
        if(data[i]==wagon){
            return i;
        }
    }
    return -1;
}

void WagonStack::clear()
{
    // Remove all wagons. The capacity does not change.
    top=-1;
}
