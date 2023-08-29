//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <memory>
#include <vector>
#include <utility>

/**
 * Storage container that stores a vector of unique pointers of T,
 * but represents most of the public facing accessors (iterators,
 * operator[]) as a vector of T.
 *
 * That is, these accessors dereference the underlying storage.
 * More importantly, if data is not properly initialized using
 * setValue(), this dereferencing will either lead to an assertion
 * or a nullptr dereference.
 */
template <class T>
class UniqueStorage
{
public:
  UniqueStorage() = default;
  UniqueStorage(UniqueStorage &&) = default;
  UniqueStorage(const UniqueStorage & mp) = delete;
  UniqueStorage & operator=(const UniqueStorage &) = delete;

  /**
   * Iterator that adds an additional dereference to BaseIterator.
   */
  template <class BaseIterator>
  struct DereferenceIterator : public BaseIterator
  {
    DereferenceIterator(const BaseIterator & it) : BaseIterator(it) {}

    using value_type = typename BaseIterator::value_type::element_type;
    using pointer = value_type *;
    using reference = value_type &;

    reference operator*() const
    {
      auto & val = BaseIterator::operator*();
      mooseAssert(val, "Null object");
      return *val;
    }
    pointer operator->() const { return BaseIterator::operator*().get(); }
    reference operator[](size_t n) const
    {
      auto & val = BaseIterator::operator[](n);
      mooseAssert(val, "Null object");
      return *val;
    }
  };

  using values_type = typename std::vector<std::unique_ptr<T>>;
  using iterator = DereferenceIterator<typename values_type::iterator>;
  using const_iterator = DereferenceIterator<typename values_type::const_iterator>;

  /**
   * Begin and end iterators to the underlying data.
   *
   * Note that dereferencing these iterators may lead to an assertion
   * or the dereference of a nullptr whether or not the underlying data
   * is initialized.
   */
  ///@{
  iterator begin() { return iterator(_values.begin()); }
  iterator end() { return iterator(_values.end()); }
  const_iterator begin() const { return const_iterator(_values.begin()); }
  const_iterator end() const { return const_iterator(_values.end()); }
  ///@}

  /**
   * @returns A reference to the underlying data at index \p i.
   *
   * Note that the underlying data may not necessarily be initialized,
   * in which case this will throw an assertion or dereference a nullptr.
   *
   * You can check whether or not the underlying data is intialized
   * with hasValue(i).
   */
  ///@{
  const T & operator[](const std::size_t i) const
  {
    mooseAssert(hasValue(i), "Null object");
    return *pointerValue(i);
  }
  T & operator[](const std::size_t i) { return const_cast<T &>(std::as_const(*this)[i]); }
  ///@}

  /**
   * @returns The size of the underlying storage.
   *
   * Note that this is not necessarily the size of _constructed_ objects,
   * as underlying objects could be uninitialized
   */
  std::size_t size() const { return _values.size(); }
  /**
   * @returns Whether or not the underlying storage is empty.
   */
  bool empty() const { return _values.empty(); }

  /**
   * @returns whether or not the underlying object at index \p is initialized
   */
  bool hasValue(const std::size_t i) const { return pointerValue(i) != nullptr; }

  /**
   * @returns A pointer to the underlying data at index \p i
   *
   * The pointer will be nullptr if !hasValue(i), that is, if the
   * unique_ptr at index \p i is not initialized
   */
  ///@{
  const T * queryValue(const std::size_t i) const { return pointerValue(i).get(); }
  T * queryValue(const std::size_t i)
  {
    return const_cast<T *>(std::as_const(*this).queryValue(i));
  }
  ///@}

protected:
  /**
   * Sets the underlying unique_ptr at index \p i to \p ptr.
   *
   * This can be used to construct objects in the storage, i.e.,
   * setPointer(0, std::make_unique<T>(...));
   *
   * This is the only method that allows for the modification of
   * ownership in the underlying vector. Protect it wisely.
   */
  void setPointer(const std::size_t i, std::unique_ptr<T> && ptr)
  {
    mooseAssert(size() > i, "Invalid size");
    _values[i] = std::move(ptr);
  }

  /**
   * Adds the given object in \p ptr to the storage.
   */
  T & addPointer(std::unique_ptr<T> && ptr)
  {
    mooseAssert(ptr, "Null object");
    return *_values.emplace_back(std::move(ptr));
  }

  /**
   * Resizes the underlying vector.
   *
   * This is the only method that allows for modification of
   * the underlying container. Protect it wisely.
   */
  void resize(const std::size_t size) { _values.resize(size); }

private:
  /**
   * Returns a read-only reference to the underlying unique pointer
   * at index \p i.
   *
   * We hope to only expose the underlying unique_ptr to this API,
   * and not in derived classes. Hopefully it can stay that way.
   */
  const std::unique_ptr<T> & pointerValue(const std::size_t i) const
  {
    mooseAssert(size() > i, "Invalid size");
    return _values[i];
  }

  /// The underlying data
  values_type _values;
};
