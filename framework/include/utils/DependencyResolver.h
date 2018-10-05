//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DEPENDENCYRESOLVER_H
#define DEPENDENCYRESOLVER_H

// MOOSE includes
#include "Moose.h"
#include "MooseError.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <exception>

template <typename T>
class DependencyResolverComparator
{
public:
  DependencyResolverComparator(const std::vector<T> & original_order)
    : _original_order(original_order)
  {
  }

  bool operator()(const T & a, const T & b) const
  {
    auto a_it = std::find(_original_order.begin(), _original_order.end(), a);
    auto b_it = std::find(_original_order.begin(), _original_order.end(), b);

    mooseAssert(a_it != _original_order.end(), "Bad DependencyResolverComparator request");
    mooseAssert(b_it != _original_order.end(), "Bad DependencyResolverComparator request");

    /**
     * Compare the iterators based on their original ordering.
     */
    return a_it < b_it;
  }

private:
  const std::vector<T> & _original_order;
};

template <typename T>
class DependencyResolver
{
public:
  DependencyResolver() {}

  ~DependencyResolver() {}

  /**
   * Insert a dependency pair - the first value or the "key" depends on the second value or the
   * "value"
   */
  void insertDependency(const T & key, const T & value);

  /**
   * Add an independent item to the set
   */
  void addItem(const T & value);

  /**
   * Clear Items from the resolver
   */
  void clear();

  /**
   * Returns a vector of sets that represent dependency resolved values.  Items in the same
   * subvector have no dependence upon one and other.
   */
  const std::vector<std::vector<T>> & getSortedValuesSets();

  /**
   * This function also returns dependency resolved values but with a simpler single vector
   * interface.
   * Some information may be lost as values at the same level that don't depend on one and other
   * can't
   * be represented in a single vector.  This isn't a problem in practice though.
   */
  const std::vector<T> & getSortedValues();

  /**
   * Return true if key depends on value.
   * That is, return true, if a chain of calls of the form
   * insertDependency(key, v0)
   * insertDependency(v0, v1)
   * insertDependency(v1, v2)
   * ...
   * insertDependency(vN, value)
   * has been performed.
   * dependsOn(x, x) always returns true
   */
  bool dependsOn(const T & key, const T & value);

  /**
   * Return true if any of elements of keys depends on value
   */
  bool dependsOn(const std::vector<T> & keys, const T & value);

  /**
   * Returns a vector of values that the given key depends on
   */
  const std::vector<T> & getValues(const T & key);

  bool operator()(const T & a, const T & b);

private:
  /**
   * Helper classes for returning only keys or values in an iterator format
   */
  template <typename map_type>
  class key_iterator;

  template <typename map_type>
  class value_iterator;

  /// This is our main data structure a multimap that contains any number of dependencies in a key = value format
  std::multimap<T, T> _depends;

  /// Used to avoid duplicate tracking of identical insertions of dependencies
  std::set<std::pair<T, T>> _unique_deps;

  /// Extra items that need to come out in the sorted list but contain no dependencies
  std::vector<T> _independent_items;

  // A vector retaining the order in which items were added to the
  // resolver, to disambiguate ordering of items with no
  // mutual interdependencies
  std::vector<T> _ordering_vector;

  /// The sorted vector of sets
  std::vector<std::vector<T>> _ordered_items;

  /// The sorted vector (if requested)
  std::vector<T> _ordered_items_vector;

  /// List of values that a given key depends upon
  std::vector<T> _values_vector;
};

template <typename T>
class CyclicDependencyException : public std::runtime_error
{
public:
  CyclicDependencyException(const std::string & error,
                            const std::multimap<T, T> & cyclic_items) throw()
    : runtime_error(error), _cyclic_items(cyclic_items)
  {
  }

  CyclicDependencyException(const CyclicDependencyException & e) throw()
    : runtime_error(e), _cyclic_items(e._cyclic_items)
  {
  }

  ~CyclicDependencyException() throw() {}

  const std::multimap<T, T> & getCyclicDependencies() const { return _cyclic_items; }

private:
  std::multimap<T, T> _cyclic_items;
};

/**
 * Helper class definitions
 */
template <typename T>
template <typename map_type>
class DependencyResolver<T>::key_iterator : public map_type::iterator
{
public:
  typedef typename map_type::iterator map_iterator;
  typedef typename map_iterator::value_type::first_type key_type;

  key_iterator(const map_iterator & other) : map_type::iterator(other){};

  key_type & operator*() { return map_type::iterator::operator*().first; }
};

template <typename T>
template <typename map_type>
class DependencyResolver<T>::value_iterator : public map_type::iterator
{
public:
  typedef typename map_type::iterator map_iterator;
  typedef typename map_iterator::value_type::second_type value_type;

  value_iterator(const map_iterator & other) : map_type::iterator(other){};

  value_type & operator*() { return map_type::iterator::operator*().second; }
};

/**
 * DependencyResolver class definitions
 */
template <typename T>
void
DependencyResolver<T>::insertDependency(const T & key, const T & value)
{
  auto k = std::make_pair(key, value);
  if (_unique_deps.count(k) > 0)
    return;
  _unique_deps.insert(k);

  if (dependsOn(value, key))
  {
    throw CyclicDependencyException<T>(
        "DependencyResolver: attempt to insert dependency will result in cyclic graph", _depends);
  }
  _depends.insert(std::make_pair(key, value));
  if (std::find(_ordering_vector.begin(), _ordering_vector.end(), key) == _ordering_vector.end())
    _ordering_vector.push_back(key);
  if (std::find(_ordering_vector.begin(), _ordering_vector.end(), value) == _ordering_vector.end())
    _ordering_vector.push_back(value);
}

template <typename T>
void
DependencyResolver<T>::addItem(const T & value)
{
  if (std::find(_independent_items.begin(), _independent_items.end(), value) ==
      _independent_items.end())
    _independent_items.push_back(value);
  if (std::find(_ordering_vector.begin(), _ordering_vector.end(), value) == _ordering_vector.end())
    _ordering_vector.push_back(value);
}

template <typename T>
void
DependencyResolver<T>::clear()
{
  _depends.clear();
  _independent_items.clear();
  _ordering_vector.clear();
  _ordered_items.clear();
  _ordered_items_vector.clear();
  _values_vector.clear();
}

template <typename T>
const std::vector<std::vector<T>> &
DependencyResolver<T>::getSortedValuesSets()
{
  // Use the original ordering for ordering subvectors
  DependencyResolverComparator<T> comp(_ordering_vector);

  /**
   * Make a copy of the map to work on since:
   * 1) we will remove values from the map
   * 2) We need the copy to be sorted in an unambiguous order.
   */
  typedef std::multimap<T, T, DependencyResolverComparator<T>> dep_multimap;
  dep_multimap depends(_depends.begin(), _depends.end(), comp);

  // Build up a set of all keys in depends that have nothing depending on them,
  // and put it in the orphans set.
  std::set<T> nodepends;

  std::set<T> all;
  std::set<T> dependees;
  for (auto & entry : depends)
  {
    dependees.insert(entry.second);
    all.insert(entry.first);
    all.insert(entry.second);
  }

  std::set<T> orphans;
  std::set_difference(all.begin(),
                      all.end(),
                      dependees.begin(),
                      dependees.end(),
                      std::inserter(orphans, orphans.end()));

  // Remove items from _independent_items if they actually appear in depends
  for (auto siter = _independent_items.begin(); siter != _independent_items.end();)
  {
    T key = *siter;
    bool founditem = false;
    for (auto i2 : depends)
    {
      if (i2.first == key || i2.second == key)
      {
        founditem = true;
        break;
      }
    }
    if (founditem)
      siter = _independent_items.erase(siter); // post increment to maintain a valid iterator
    else
      ++siter;
  }

  /* Clear the ordered items vector */
  _ordered_items.clear();

  // Put the independent items into the first set in _ordered_items
  std::vector<T> next_set(_independent_items.begin(), _independent_items.end());

  /* Topological Sort */
  while (!depends.empty())
  {
    /* Work with sets since set_difference doesn't always work properly with multi_map due
     * to duplicate keys
     */
    std::set<T, DependencyResolverComparator<T>> keys(
        typename DependencyResolver<T>::template key_iterator<dep_multimap>(depends.begin()),
        typename DependencyResolver<T>::template key_iterator<dep_multimap>(depends.end()),
        comp);

    std::set<T, DependencyResolverComparator<T>> values(
        typename DependencyResolver<T>::template value_iterator<dep_multimap>(depends.begin()),
        typename DependencyResolver<T>::template value_iterator<dep_multimap>(depends.end()),
        comp);

    std::vector<T> current_set(next_set);
    next_set.clear();

    /* This set difference creates a set of items that have no dependencies in the depend map*/
    std::set<T, DependencyResolverComparator<T>> difference(comp);

    std::set_difference(values.begin(),
                        values.end(),
                        keys.begin(),
                        keys.end(),
                        std::inserter(difference, difference.end()),
                        comp);

    /* Now remove items from the temporary map that have been "resolved" */
    if (!difference.empty())
    {
      for (auto iter = depends.begin(); iter != depends.end();)
      {
        if (difference.find(iter->second) != difference.end())
        {
          T key = iter->first;
          depends.erase(iter++); // post increment to maintain a valid iterator

          // If the item is at the end of a dependency chain (by being an orphan) AND
          // is not still in the depends map because it still has another unresolved link
          // insert it into the next_set
          if (orphans.find(key) != orphans.end() && depends.find(key) == depends.end())
            next_set.push_back(key);
        }
        else
          ++iter;
      }
      /* Add the current set of resolved items to the ordered vector */
      current_set.insert(current_set.end(), difference.begin(), difference.end());
      _ordered_items.push_back(current_set);
    }
    else
    {

      /* If the last set difference was empty but there are still items that haven't come out then
       * there is
       * a cyclic dependency somewhere in the map
       */
      if (!depends.empty())
      {
        std::ostringstream oss;
        oss << "Cyclic dependency detected in the Dependency Resolver.  Remaining items are:\n";
        for (auto j : depends)
          oss << j.first << " -> " << j.second << "\n";
        // Return a multimap without a weird comparator, to avoid
        // dangling reference problems and for backwards compatibility
        std::multimap<T, T> cyclic_deps(depends.begin(), depends.end());
        throw CyclicDependencyException<T>(oss.str(), cyclic_deps);
      }
    }
  }

  if (next_set.empty())
  {
    if (!_independent_items.empty() || !depends.empty())
      mooseError("DependencyResolver error: next_set shouldn't be empty!");
  }
  else
  {
    _ordered_items.push_back(next_set);
  }

  return _ordered_items;
}

template <typename T>
const std::vector<T> &
DependencyResolver<T>::getSortedValues()
{
  _ordered_items_vector.clear();

  getSortedValuesSets();

  for (auto subset : _ordered_items)
    std::copy(subset.begin(), subset.end(), std::back_inserter(_ordered_items_vector));

  return _ordered_items_vector;
}

template <typename T>
bool
DependencyResolver<T>::dependsOn(const T & key, const T & value)
{
  if (key == value)
    return true;

  // recursively call dependsOn on all the things that key depends on
  std::pair<typename std::multimap<T, T>::iterator, typename std::multimap<T, T>::iterator> ret;
  ret = _depends.equal_range(key);
  for (typename std::multimap<T, T>::iterator it = ret.first; it != ret.second; ++it)
    if (dependsOn(it->second, value))
      return true;

  // No dependencies were found,
  // or the key is not in the tree (so it has no dependencies).
  // In this latter case, the only way that key depends on value is if key == value,
  // but we've already checked that
  return false;
}

template <typename T>
bool
DependencyResolver<T>::dependsOn(const std::vector<T> & keys, const T & value)
{
  for (auto key : keys)
    if (dependsOn(key, value))
      return true;
  return false;
}

template <typename T>
const std::vector<T> &
DependencyResolver<T>::getValues(const T & key)
{
  _values_vector.clear();

  std::pair<typename std::multimap<T, T>::iterator, typename std::multimap<T, T>::iterator> ret;
  ret = _depends.equal_range(key);

  for (typename std::multimap<T, T>::iterator it = ret.first; it != ret.second; ++it)
    _values_vector.push_back(it->second);

  return _values_vector;
}

template <typename T>
bool
DependencyResolver<T>::operator()(const T & a, const T & b)
{
  if (_ordered_items_vector.empty())
    getSortedValues();

  typename std::vector<T>::const_iterator a_it =
      std::find(_ordered_items_vector.begin(), _ordered_items_vector.end(), a);
  typename std::vector<T>::const_iterator b_it =
      std::find(_ordered_items_vector.begin(), _ordered_items_vector.end(), b);

  /**
   * It's possible that a and/or b are not in the resolver in which case
   *  we want those values to come out first.  However, we need to make
   *  sure that we maintain strict weak ordering so we'll compare b_it first,
   *  which will return false for a_it < b_it and b_it < a_it when both values
   *  are not in the ordered_items vector.
   */
  if (b_it == _ordered_items_vector.end())
    return false;
  if (a_it == _ordered_items_vector.end())
    return true;
  else
    /**
     * Compare the iterators.  Users sometime fail to state all their
     * items' dependencies, but do introduce dependant items only after
     * the items they depended on; this preserves that sorting.
     */
    return a_it < b_it;
}

#endif // DEPENDENCYRESOLVER_H
