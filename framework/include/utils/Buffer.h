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
   * Resize the capacity
   */
  void setCapacity(const std::size_t capacity) { this->_data.resize(capacity); }
  /**
   * Get the capacity
   */
  std::size_t capacity() const { return this->_data.size(); }

  /**
   * Set the size
   */
  void setSize(const std::size_t size) { this->_end_pos = this->newEnd(this->_begin_pos + size); }
  /**
   * Get the size
   */
  std::size_t size() const { return this->_end_pos - this->_begin_pos; }

  /**
   * Whether or not the buffer is empty
   */
  bool empty() const { return !size(); }

  /**
   * Add a new entry on the end
   */
  void push_back(const T & value);

  /**
   * Moves the object into the buffer (calls std::move())
   */
  void move(T & value);

  /**
   * Add new entries to the end
   *
   * Everything in [in_begin, in_end) is appended
   */
  void append(const_iterator in_begin, const_iterator in_end);
  /**
   * Add new entries to the end
   */
  void append(const std::vector<T> & vals);

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
  /**
   * Find out where the new end will be.
   * This will resize/copy data as necessary
   *
   * @param new_end the proposed new_end position
   * @return The actual position of the new ending
   */
  virtual std::size_t newEnd(const std::size_t new_end) = 0;

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
Buffer<T>::Buffer(const std::size_t capacity) : _data(capacity), _begin_pos(0), _end_pos(0)
{
}

template <typename T>
void
Buffer<T>::push_back(const T & value)
{
  this->_end_pos = newEnd(this->_end_pos + 1);
  this->_data[this->_end_pos - 1] = value;
}

template <typename T>
void
Buffer<T>::move(T & value)
{
  this->_end_pos = newEnd(this->_end_pos + 1);
  this->_data[this->_end_pos - 1] = std::move(value);
}

template <typename T>
void
Buffer<T>::append(const_iterator in_begin, const_iterator in_end)
{
  const auto additional_size = std::distance(in_begin, in_end);
  if (additional_size == 0)
    return;

  this->_end_pos = this->newEnd(this->_end_pos + additional_size);
  std::copy(in_begin, in_end, this->end() - additional_size);
}

template <typename T>
void
Buffer<T>::append(const std::vector<T> & vals)
{
  this->append(vals.begin(), vals.end());
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
