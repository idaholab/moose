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

typedef std::multimap<std::string, std::string> depends_map;

class DependencyResolver
{
public:
  DependencyResolver() {}

  ~DependencyResolver() {}
  
  /**
   * Insert a dependecy pair - the first value or the "key" depends on the second value or the "value"
   */
  void insertDependency(const std::string & key, const std::string & value);

  /**
   * Add an independent item to the set
   */
  void addItem(const std::string & value);
  
  /**
   * Returns a reference to a sorted vector of values
   */
  const std::vector<std::string> & getSortedValues();

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
  depends_map _depends;

  /**
   * Extra items that need to come out in the sorted list but contain no dependencies
   */
  std::set<std::string> _independent_items;

  /**
   * The sorted vector
   */
  std::vector<std::string> _ordered_items;
};

#endif
