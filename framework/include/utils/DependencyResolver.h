/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DEPENDENCYRESOLVER_H
#define DEPENDENCYRESOLVER_H

#include "Moose.h"
#include "MooseError.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <exception>

template <typename T>
class DependencyResolver
{
public:
  DependencyResolver() {}

  ~DependencyResolver() {}

  /**
   * Insert a dependency pair - the first value or the "key" depends on the second value or the "value"
   */
  void insertDependency(const T & key, const T & value);

  /**
   * Add an independent item to the set
   */
  void addItem(const T & value);

  /**
   * Returns a vector of sets that represent dependency resolved values.  Items in the same
   * set have no dependence upon one and other.
   */
  const std::vector<std::set<T> > & getSortedValuesSets();

  /**
   * This function also returns dependency resolved values but with a simpler single vector interface.
   * Some information may be lost as values at the same level that don't depend on one and other can't
   * be represented in a single vector.  This isn't a problem in practice though.
   */
  const std::vector<T> & getSortedValues();

  bool operator() (const T & a, const T & b);

private:
  /**
   * Helper classes for returning only keys or values in an iterator format
   */
  template<typename map_type>
  class key_iterator;

  template<typename map_type>
  class value_iterator;

  /// This is our main data structure a multimap that contains any number of dependencies in a key = value format
  std::multimap<T, T> _depends;

  /// Extra items that need to come out in the sorted list but contain no dependencies
  std::set<T> _independent_items;

  /// The sorted vector of sets
  std::vector<std::set<T> > _ordered_items;

  /// The sorted vector (if requested)
  std::vector<T> _ordered_items_vector;
};

template<typename T>
class CyclicDependencyException : public std::runtime_error
{
public:
  CyclicDependencyException(const std::string &error, const std::multimap<T, T> & cyclic_items) throw() :
      runtime_error(error),
      _cyclic_items(cyclic_items)
    {
    }

  CyclicDependencyException(const CyclicDependencyException & e) throw() :
      runtime_error(e),
      _cyclic_items(e._cyclic_items)
    {
    }

  ~CyclicDependencyException() throw()
    {
    }

  const std::multimap<T, T> & getCyclicDependencies() const
    {
      return _cyclic_items;
    }

private:
  std::multimap<T, T> _cyclic_items;
};

/**
 * Helper class definitions
 */
template <typename T>
template<typename map_type>
class DependencyResolver<T>::key_iterator : public map_type::iterator
{
public:
  typedef typename map_type::iterator map_iterator;
  typedef typename map_iterator::value_type::first_type key_type;

  key_iterator(const map_iterator& other) : map_type::iterator(other) {} ;

  key_type& operator *()
    {
      return map_type::iterator::operator*().first;
    }
};

template <typename T>
template<typename map_type>
class DependencyResolver<T>::value_iterator : public map_type::iterator
{
public:
  typedef typename map_type::iterator map_iterator;
  typedef typename map_iterator::value_type::second_type value_type;

  value_iterator(const map_iterator& other) : map_type::iterator(other) {} ;

  value_type& operator *()
    {
      return map_type::iterator::operator*().second;
    }
};


/**
 * DependencyResolver class definitions
 */
template <typename T>
void
DependencyResolver<T>::insertDependency(const T & key, const T & value)
{
  _depends.insert(std::make_pair(key, value));
}

template <typename T>
void
DependencyResolver<T>::addItem(const T & value)
{
  _independent_items.insert(value);
}

template <typename T>
const std::vector<std::set<T> > &
DependencyResolver<T>::getSortedValuesSets()
{
  /* Make a copy of the map to work on since we will remove values from the map*/
  std::multimap<T, T> depends = _depends;

  //Build up a set of all keys in depends that have nothing depending on them,
  //and put it in the nodepends set.  These are the leaves of the dependency tree.
  std::set<T> nodepends;
  for (typename std::multimap<T, T>::iterator i = depends.begin(); i != depends.end(); ++i)
  {
    T key=i->first;
    bool founditem=false;
    for (typename std::multimap<T, T>::iterator i2 = depends.begin(); i2 != depends.end(); ++i2)
    {
      if (i2->second == key)
      {
        founditem = true;
        break;
      }
    }
    if (!founditem)
      nodepends.insert(key);
  }

  //Remove items from _independent_items if they actually appear in depends
  for (typename std::set<T>::iterator siter = _independent_items.begin(); siter != _independent_items.end();)
  {
    T key=*siter;
    bool founditem=false;
    for (typename std::multimap<T, T>::iterator i2 = depends.begin(); i2 != depends.end(); ++i2)
    {
      if (i2->first == key || i2->second == key)
      {
        founditem = true;
        break;
      }
    }
    if (founditem)
      _independent_items.erase(siter++); // post increment to maintain a valid iterator
    else
      ++siter;
  }

  /* Clear the ordered items vector */
  _ordered_items.clear();

  //Put the independent items into the first set in _ordered_items
  std::set<T> next_set = _independent_items;

  /* Topological Sort */
  while (!depends.empty())
  {
    std::set<T> keys, values, difference, current_set;

    /* Work with sets since set_difference doesn't always work properly with multi_map due
     * to duplicate keys
     */
    std::copy(typename DependencyResolver<T>::template key_iterator<std::multimap<T, T> >(depends.begin()),
              typename DependencyResolver<T>::template key_iterator<std::multimap<T, T> >(depends.end()),
              std::inserter(keys, keys.end()));

    std::copy(typename DependencyResolver<T>::template value_iterator<std::multimap<T, T> >(depends.begin()),
              typename DependencyResolver<T>::template value_iterator<std::multimap<T, T> >(depends.end()),
              std::inserter(values, values.end()));

    current_set.clear();
    current_set.insert(next_set.begin(), next_set.end());
    next_set.clear();

    /* This set difference creates a set of items that have no dependencies in the depend map*/
    std::set_difference(values.begin(), values.end(), keys.begin(), keys.end(),
                        std::inserter(difference, difference.end()));

    /* Now remove items from the temporary map that have been "resolved" */
    if (!difference.empty())
    {
      for (typename std::multimap<T, T>::iterator iter = depends.begin(); iter != depends.end();)
      {
        if (difference.find(iter->second) != difference.end())
        {
          T key = iter->first;
          depends.erase(iter++);   // post increment to maintain a valid iterator

          // If the item is at the end of a dependency chain (by being in nodepends) AND
          // is not still in the depends map because it still has another unresolved link
          // insert it into the next_set
          if (nodepends.find(key) != nodepends.end() && depends.find(key) == depends.end())
            next_set.insert(key);
        }
        else
          ++iter;
      }
      /* Add the current set of resolved items to the ordered vector */
      current_set.insert(difference.begin(), difference.end());
      _ordered_items.push_back(current_set);
    }
    else
    {

    /* If the last set difference was empty but there are still items that haven't come out then there is
     * a cyclic dependency somewhere in the map
     */
      if (!depends.empty())
      {
        std::ostringstream oss;
        oss << "Cyclic dependency detected in the Dependency Resolver.  Remaining items are:\n";
        for (typename std::multimap<T, T>::iterator j = depends.begin(); j != depends.end(); ++j)
          oss << j->first << " -> " << j->second << "\n";
        throw CyclicDependencyException<T>(oss.str(), depends);
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

  typename std::vector<std::set<T> >::iterator iter = _ordered_items.begin();
  for ( ; iter != _ordered_items.end(); ++iter)
    std::copy(iter->begin(), iter->end(), std::back_inserter(_ordered_items_vector));

  return _ordered_items_vector;
}

template <typename T>
bool
DependencyResolver<T>::operator() (const T & a, const T & b)
{
  if (_ordered_items_vector.empty())
    getSortedValues();

  typename std::vector<T>::const_iterator a_it = std::find(_ordered_items_vector.begin(), _ordered_items_vector.end(), a);
  typename std::vector<T>::const_iterator b_it = std::find(_ordered_items_vector.begin(), _ordered_items_vector.end(), b);

  /**
   * It's possible that a and/or b are not in the resolver in which case
   *  we want those values to come out first.  However, we need to make
   *  sure that we maintain strict weak ordering so we'll compare b_it first,
   *  which will return false for a_it < b_it and b_it < a_it when both values
   *  are not in the oredered_items vector.
   */
  if (b_it == _ordered_items_vector.end())
    return false;
  if (a_it == _ordered_items_vector.end())
    return true;
  else
    return a_it < b_it;  // Yes - compare the iterators...
}

#endif // DEPENDENCYRESOLVER_H
