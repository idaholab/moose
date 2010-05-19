#ifndef MOOSEARRAY_H
#define MOOSEARRAY_H

#include "Moose.h"

template<typename T>
class MooseArray
{
public:
  /**
   * Default constructor.  Doesn't initialize anything.
   */
  MooseArray();

  /**
   * @param size The initial size of the array.
   */
  MooseArray(const unsigned int size);

  /**
   * Manually deallocates the data pointer
   */
  void release();
  
  /**
   * Change the number of elements the array can store.
   *
   * Will allocate more memory if necessary.
   *
   * Can destroy data currently in array!
   * Basically, data retention not guaranteed.
   *
   * Note that this does _not_ free unused memory.
   * This is done for speed.
   */
  void resize(const unsigned int size);

  /**
   * The number of elements that can currently
   * be stored in the array.
   */
  unsigned int size();

  /**
   * Get element i out of the array.
   */
  T & operator[](const unsigned int i);

  /**
   * Doesn't actually make a copy of the data.
   *
   * Just makes _this_ object operate on the same data.
   *
   * Note! You can leak memory with this function if you
   * don't take care to have a copy of _this_ array somewhere
   * else.  This is because the data pointer will get overriden
   * here.
   */
  void shallowCopy(const MooseArray & rhs);

private:

  /**
   * Actual data pointer.
   */
  T * _data;

  /**
   * The current number of elements the array can hold.
   */
  unsigned int _size;

  /**
   * Number of allocated memory positions for storage.
   */
  unsigned int _allocated_size;  
};

template<typename T>
inline
void
MooseArray<T>::resize(const unsigned int size)
{
  if(size <= _allocated_size)
    _size = size;
  else
  {
    T * new_pointer = new T[size];
    mooseAssert(new_pointer, "Failed to allocate MooseArray memory!");

    if (_data != NULL)
      delete [] _data;
    _data = new_pointer;
    _allocated_size = size;
    _size = size;
  }
}

template<typename T>
inline
unsigned int
MooseArray<T>::size()
{
  return _size;
}

template<typename T>
inline
T &
MooseArray<T>::operator[](const unsigned int i)
{
  mooseAssert(i < _size, "Access out of bounds in MooseArray!");
  
  return _data[i];
}

template<typename T>
inline
void
MooseArray<T>::shallowCopy(const MooseArray & rhs)
{
  _data = rhs._data;
  _size = rhs._size;
  _allocated_size = rhs._allocated_size;
}

#endif //MOOSEARRAY_H

