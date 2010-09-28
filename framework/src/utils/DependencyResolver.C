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

#include "DependencyResolver.h"

#include <algorithm>
#include "Moose.h"

/**
 * Helper class definitions
 */
template<typename map_type>
class DependencyResolver::key_iterator : public map_type::iterator
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

template<typename map_type>
class DependencyResolver::value_iterator : public map_type::iterator
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
void
DependencyResolver::insertDependency(const std::string & key, const std::string & value)
{
  _depends.insert(std::make_pair(key, value));
}

void
DependencyResolver::addItem(const std::string & value)
{
  _independent_items.insert(value);
}

const std::vector<std::string> &
DependencyResolver::getSortedValues()
{
  /* Make a copy of the map to work on since we will remove values from the map*/
  depends_map depends = _depends;

  /* Add in the independent values */
  for (std::set<std::string>::iterator i = _independent_items.begin(); i != _independent_items.end(); ++i)
    depends.insert(std::make_pair("", *i));

  /* Add in the key values to make sure we have all values in our starting set */
  for (depends_map::iterator i = _depends.begin(); i != _depends.end(); ++i)
    depends.insert(std::make_pair("", i->first));

  /* Clear the ordered items vector */
  _ordered_items.clear();

  /* Topological Sort */
  while (!depends.empty())
  {
    std::set<std::string> keys, values, difference;

    /* We have to work with sets since the set_difference algorithm doesn't always work properly with multi_map due
     * to duplicate keys
     */
    std::copy(DependencyResolver::key_iterator<depends_map>(depends.begin()),
              DependencyResolver::key_iterator<depends_map>(depends.end()),
              std::inserter(keys, keys.end()));
    
    std::copy(DependencyResolver::value_iterator<depends_map>(depends.begin()),
              DependencyResolver::value_iterator<depends_map>(depends.end()),
              std::inserter(values, values.end()));

    /* This set difference creates a set of items that have no dependencies in the depend map*/
    std::set_difference(values.begin(), values.end(), keys.begin(), keys.end(), 
                        std::inserter(difference, difference.end()));

    /* If the last set difference was empty but there are still items that haven't come out then there is
     * a cyclic dependency somewhere in the map
     */
    if (difference.empty())
      mooseError("Cyclic dependency detected in the Dependency Resolver");

    /* Now remove items from the temporary map that have been "resolved" */
    for (depends_map::iterator iter = depends.begin(); iter != depends.end();)
    {
      if (difference.find(iter->second) != difference.end())
        depends.erase(iter++);   // post increment is required here to maintain a valid iterator
      else
        ++iter;
    }

    /* Add the current set of resolved items to the ordered vector */
    std::copy(difference.begin(), difference.end(), std::back_inserter(_ordered_items));
  }

  return _ordered_items;
}




