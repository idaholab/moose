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

#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#include "Moose.h"


template <typename T>
class DependencyResolver
{
public:
  DependencyResolver() {}

  ~DependencyResolver() {}
  
  /**
   * Insert a dependecy pair - the first value or the "key" depends on the second value or the "value"
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
   * This function also returns dependency resolved values but with a simplier single vector interface.
   * Some information may be lost as values at the same level that don't depend on one and other can't
   * be represented in a single vector.  This isn't a problem in practice though.
   */
  const std::vector<T> & getSortedValues();


private:
  /**
   * Helper classes for returning only keys or values in an iterator format
   */
  template<typename map_type>
  class key_iterator;

  template<typename map_type>
  class value_iterator;
  
  /**
   * This is our main datastructure a multimap that contains any number of dependencies in a key = value format
   */
  std::multimap<T, T> _depends;

  /**
   * Extra items that need to come out in the sorted list but contain no dependencies
   */
  std::set<T> _independent_items;

  /**
   * The sorted vector of sets
   */
  std::vector<std::set<T> > _ordered_items;

  /**
   * The sorted vector (if requested)
   */
  std::vector<T> _ordered_items_vector;
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

  T t; // empty
  
  /* Add in the independent values */
  for (typename std::set<T>::iterator i = _independent_items.begin(); i != _independent_items.end(); ++i)
    depends.insert(std::make_pair(t, *i));

  /* Add in the key values to make sure we have all values in our starting set */
  for (typename std::multimap<T, T>::iterator i = _depends.begin(); i != _depends.end(); ++i)
    depends.insert(std::make_pair(t, i->first));

  /* Clear the ordered items vector */
  _ordered_items.clear();

  /* Topological Sort */
  while (!depends.empty())
  {
    std::set<T> keys, values, difference;

    /* We have to work with sets since the set_difference algorithm doesn't always work properly with multi_map due
     * to duplicate keys
     */
    std::copy(typename DependencyResolver<T>::template key_iterator<std::multimap<T, T> >(depends.begin()),
              typename DependencyResolver<T>::template key_iterator<std::multimap<T, T> >(depends.end()),
              std::inserter(keys, keys.end()));
    
    std::copy(typename DependencyResolver<T>::template value_iterator<std::multimap<T, T> >(depends.begin()),
              typename DependencyResolver<T>::template value_iterator<std::multimap<T, T> >(depends.end()),
              std::inserter(values, values.end()));

    /* This set difference creates a set of items that have no dependencies in the depend map*/
    std::set_difference(values.begin(), values.end(), keys.begin(), keys.end(), 
                        std::inserter(difference, difference.end()));

    /* If the last set difference was empty but there are still items that haven't come out then there is
     * a cyclic dependency somewhere in the map
     */
    if (difference.empty())
    {
      std::ostringstream oss;
      oss << "Cyclic dependency detected in the Dependency Resolver.  Remaining items are:\n";
      for (typename std::multimap<T, T>::iterator j = depends.begin(); j != depends.end(); ++j)
        oss << j->first << " -> " << j->second << "\n";
      mooseError(oss.str());
    }
    
    

    /* Now remove items from the temporary map that have been "resolved" */
    for (typename std::multimap<T, T>::iterator iter = depends.begin(); iter != depends.end();)
    {
      if (difference.find(iter->second) != difference.end())
        depends.erase(iter++);   // post increment is required here to maintain a valid iterator
      else
        ++iter;
    }

    /* Add the current set of resolved items to the ordered vector */
    _ordered_items.push_back(difference);
    //std::copy(difference.begin(), difference.end(), std::back_inserter(_ordered_items));
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
  
#endif // DEPENDENCYRESOLVER_H
