//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Buffer.h"

namespace MooseUtils
{

/**
 * An optimized circular buffer.
 *
 * This also always ensures that begin() < end().
 * That means that if the end of the buffer capacity is reached, an O(N) operation
 * will be used to move data from the end of the capacity to the beginning
 *
 * This is done so that begin()/end() iterators can be used in OpenMP loops
 * Also means that operator[] works sequentially between 0 and size()
 *
 * It will also automatically grow larger if capacity is reached
 *
 * NOTE: This buffer will not properly wrap around as a standard circular buffer
 * does. Once the end of the internal storage has been reached, the data
 * will be moved to the beginning of the internal storage via a copy. This is
 * needed to ensure that the access through the begin()/end() iterators
 * is contiguous.
 */
template <typename T>
class CircularBuffer : public Buffer<T>
{
public:
  CircularBuffer();

  CircularBuffer(const std::size_t capacity);

  void erase(const std::size_t num) override;
  void eraseChunk(const std::size_t chunk_size) override;

  typename Buffer<T>::iterator beginChunk(const std::size_t chunk_size) override;
  typename Buffer<T>::const_iterator beginChunk(const std::size_t chunk_size) const override;
  typename Buffer<T>::iterator endChunk(const std::size_t chunk_size) override;
  typename Buffer<T>::const_iterator endChunk(const std::size_t chunk_size) const override;

  void emplaceBack(T && value) override;
};

template <typename T>
CircularBuffer<T>::CircularBuffer() : Buffer<T>()
{
}

template <typename T>
CircularBuffer<T>::CircularBuffer(const std::size_t capacity) : Buffer<T>(capacity)
{
}

template <typename T>
void
CircularBuffer<T>::erase(const std::size_t num)
{
  mooseAssert(num <= this->size(), "Cannot erase past the last entry");

  this->_begin_pos += num;

  // If there's nothing in the buffer - let's reset the positions
  if (this->_begin_pos == this->_end_pos)
    this->clear();
}

template <typename T>
void
CircularBuffer<T>::eraseChunk(const std::size_t chunk_size)
{
  if (chunk_size > this->size())
    this->erase(this->size());
  else
    this->erase(chunk_size);
}

template <typename T>
typename Buffer<T>::iterator
CircularBuffer<T>::beginChunk(const std::size_t /* chunk_size */)
{
  return this->begin();
}

template <typename T>
typename Buffer<T>::const_iterator
CircularBuffer<T>::beginChunk(const std::size_t /* chunk_size */) const
{
  return this->begin();
}

template <typename T>
typename Buffer<T>::iterator
CircularBuffer<T>::endChunk(const std::size_t chunk_size)
{
  if (chunk_size > this->size())
    return this->end();
  else
    return this->begin() + chunk_size;
}

template <typename T>
typename Buffer<T>::const_iterator
CircularBuffer<T>::endChunk(const std::size_t chunk_size) const
{
  if (chunk_size > this->size())
    return this->end();
  else
    return this->begin() + chunk_size;
}

template <typename T>
void
CircularBuffer<T>::emplaceBack(T && value)
{
  // Vector does not have space for the new value
  if (this->_end_pos + 1 > this->_data.size())
  {
    // Reserve some more while we're here so we can avoid reallocating later
    this->reserve(2 * (this->_end_pos + 1));

    // If we're at the beginning, just emplace back
    if (this->_begin_pos == 0)
      this->_data.emplace_back(std::move(value));
    // Begin isn't at the beginning, so shift everything while we're here
    else
    {
      auto to_it = this->_data.begin();
      for (auto from_it = this->begin(); from_it != this->end(); ++from_it)
        *to_it++ = std::move(*from_it);

      // Shift back end position based on everything we moved to the beginning
      this->_end_pos -= this->_begin_pos;
      // Move the value
      this->_data[this->_end_pos] = std::move(value);
      // Begin is now at the vector beginning
      this->_begin_pos = 0;
    }
  }
  // Vector currently has space
  else
    this->_data[this->_end_pos] = std::move(value);

  ++this->_end_pos;
}

}
