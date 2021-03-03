//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"

#include <vector>

namespace MooseUtils
{

/**
 * Base class for a buffer.
 *
 * Enables the controlled access of an underlying raw vector
 * for storage, which can be used to limit memory allocation
 * and copies for various buffer types while providing a
 * useful public API.
 */
template <typename T>
class Buffer
{
public:
  typedef typename std::vector<T>::iterator iterator;
  typedef typename std::vector<T>::const_iterator const_iterator;

  /**
   * Create an empty buffer
   */
  Buffer();

  /**
   * Create a buffer with a specific capacity
   */
  Buffer(const std::size_t capacity);

  virtual ~Buffer() {}

  /**
   * Reserve in the buffer
   */
  void reserve(const std::size_t capacity) { this->_data.reserve(capacity); }
  /**
   * Get the capacity of the buffer
   */
  std::size_t capacity() const { return this->_data.capacity(); }

  /**
   * Get the current size of the buffer
   */
  std::size_t size() const { return this->_end_pos - this->_begin_pos; }

  /**
   * Whether or not the buffer is empty
   */
  bool empty() const { return this->_begin_pos == this->_end_pos; }

  /**
   * Emplaces an object into the buffer
   */
  virtual void emplaceBack(T && value) = 0;

  /**
   * Remove all entries (does not change the capacity)
   * Note: this does NOT at all free any entries
   */
  void clear();

  /**
   * Remove the first num elements
   *
   * Note that erased items are not guaranteed to be freed immediately
   */
  virtual void erase(const std::size_t num) = 0;
  /*
   * Similar to erase(), but if the chunk size is larger than the current size,
   * no error (all elements are erased gracefully).
   *
   * Note that erased items are not guaranteed to be freed immediately
   */
  virtual void eraseChunk(const std::size_t chunk_size) = 0;

  /**
   * Iterator for the first entry in the buffer
   */
  iterator begin() { return this->_data.begin() + this->_begin_pos; }
  /**
   * Const iterator for the first entry in the buffer
   */
  const_iterator begin() const { return this->_data.begin() + this->_begin_pos; }

  /**
   * Iterator for the last entry in the buffer
   */
  iterator end() { return this->_data.begin() + this->_end_pos; }
  /**
   * Const iterator for the last entry in the buffer
   */
  const_iterator end() const { return this->_data.begin() + this->_end_pos; }

  /**
   * Iterator for the first entry with a given chunk size in the buffer
   * If chunk_size is greater than the size of the buffer, the full range will be given.
   */
  virtual iterator beginChunk(const std::size_t chunk_size) = 0;
  /**
   * Const iterator for the first entry with a given chunk size in the buffer
   * If chunk_size is greater than the size of the buffer, the full range will be given.
   */
  virtual const_iterator beginChunk(const std::size_t chunk_size) const = 0;

  /**
   * Iterator for the last entry of a chunk size in the buffer
   * If chunk_size is greater than the size of the buffer, the full range will be given.
   */
  virtual iterator endChunk(const std::size_t chunk_size) = 0;
  /**
   * Const iterator for the last entry of a chunk size in the buffer
   * If chunk_size is greater than the size of the buffer, the full range will be given.
   */
  virtual const_iterator endChunk(const std::size_t chunk_size) const = 0;

  /**
   * Access an entry at index
   */
  T & operator[](const std::size_t index);
  /**
   * Const access an entry at an index
   */
  const T & operator[](const std::size_t index) const;

  /**
   * Use in_data as our data vector
   */
  void swap(std::vector<T> & in_data);

  /**
   * Access the raw underlying storage.
   *
   * This is considered an advanced interface. Typically, you
   * should use the begin(), beginChunk(), end(), and endChunk()
   * methods for accessing the data. This should really only
   * be used in unit tests for verifying the underlying storage.
   */
  const std::vector<T> & data() { return _data; }

  /**
   * The current beginning position of the buffer in data().
   *
   * This is considered an advanced interface because data()
   * is the internal storage for the buffer. It should really
   * only be used in unit tests for verifying the underlying
   * storage.
   */
  std::size_t dataBeginPos() const { return _begin_pos; }

  /**
   * The current end position of the buffer in data().
   *
   * This is considered an advanced interface because data()
   * is the internal storage for the buffer. It should really
   * only be used in unit tests for verifying the underlying
   * storage.
   */
  std::size_t dataEndPos() const { return _end_pos; }

protected:
  /// The raw data
  std::vector<T> _data;

  /// The beginning position
  std::size_t _begin_pos;
  /// The ending position
  std::size_t _end_pos;
};

template <typename T>
Buffer<T>::Buffer() : _begin_pos(0), _end_pos(0)
{
}

template <typename T>
Buffer<T>::Buffer(const std::size_t capacity) : _begin_pos(0), _end_pos(0)
{
  this->reserve(capacity);
}

template <typename T>
void
Buffer<T>::clear()
{
  this->_begin_pos = 0;
  this->_end_pos = 0;
}

template <typename T>
T &
Buffer<T>::operator[](const std::size_t index)
{
  mooseAssert(this->_begin_pos + index < this->_end_pos, "Attempt to access off end of Buffer!");
  return this->_data[this->_begin_pos + index];
}

template <typename T>
const T &
Buffer<T>::operator[](const std::size_t index) const
{
  mooseAssert(this->_begin_pos + index < this->_end_pos, "Attempt to access off end of Buffer!");
  return this->_data[this->_begin_pos + index];
}

template <typename T>
void
Buffer<T>::swap(std::vector<T> & in_data)
{
  std::swap(in_data, _data);
  this->_begin_pos = 0;
  this->_end_pos = this->_data.size();
}

}
