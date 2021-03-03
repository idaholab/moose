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
 * An optimized LIFO (Last In First Out) buffer.
 *
 * Begin()/end() iterators can be used in OpenMP loops
 * Also operator[] works sequentially between 0 and size()
 *
 * It will also automatically grow larger if capacity is reached
 */
template <typename T>
class LIFOBuffer : public Buffer<T>
{
public:
  /**
   * Create an empty LIFO buffer
   */
  LIFOBuffer();

  LIFOBuffer(const std::size_t capacity);

  void erase(const std::size_t num) override;
  void eraseChunk(const std::size_t chunk_size) override;

  typename Buffer<T>::iterator beginChunk(const std::size_t chunk_size) override;
  typename Buffer<T>::const_iterator beginChunk(const std::size_t chunk_size) const override;
  typename Buffer<T>::iterator endChunk(const std::size_t chunk_size) override;
  typename Buffer<T>::const_iterator endChunk(const std::size_t chunk_size) const override;

  void emplaceBack(T && value) override;

protected:
};

template <typename T>
LIFOBuffer<T>::LIFOBuffer() : Buffer<T>()
{
}

template <typename T>
LIFOBuffer<T>::LIFOBuffer(const std::size_t capacity) : Buffer<T>(capacity)
{
}

template <typename T>
void
LIFOBuffer<T>::erase(const std::size_t num)
{
  mooseAssert(num <= this->size(), "Cannot erase past the last entry");

  this->_end_pos -= num;

  // If there's nothing in the buffer - let's reset the positions
  if (this->_begin_pos == this->_end_pos)
    this->clear();
}

template <typename T>
void
LIFOBuffer<T>::eraseChunk(const std::size_t chunk_size)
{
  if (chunk_size > this->size())
    this->erase(this->size());
  else
    this->erase(chunk_size);
}

template <typename T>
typename Buffer<T>::iterator
LIFOBuffer<T>::beginChunk(const std::size_t chunk_size)
{
  if (chunk_size > this->size())
    return this->begin();
  else
    return this->end() - chunk_size;
}

template <typename T>
typename Buffer<T>::const_iterator
LIFOBuffer<T>::beginChunk(const std::size_t chunk_size) const
{
  if (chunk_size > this->size())
    return this->begin();
  else
    return this->end() - chunk_size;
}

template <typename T>
typename Buffer<T>::iterator
LIFOBuffer<T>::endChunk(const std::size_t /* chunk_size */)
{
  return this->end();
}

template <typename T>
typename Buffer<T>::const_iterator
LIFOBuffer<T>::endChunk(const std::size_t /* chunk_size */) const
{
  return this->end();
}

template <typename T>
void
LIFOBuffer<T>::emplaceBack(T && value)
{
  if (++this->_end_pos > this->_data.size())
  {
    this->reserve(2 * this->_end_pos);
    this->_data.emplace_back(std::move(value));
  }
  else
    this->_data[this->_end_pos - 1] = std::move(value);
}

}
