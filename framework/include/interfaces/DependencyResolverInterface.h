//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// STL includes
#include <string>
#include <set>
#include <iostream>
#include <algorithm>

// MOOSE includes
#include "DependencyResolver.h"
#include "MooseUtils.h"

/**
 * Interface for sorting dependent vectors of objects.
 */
class DependencyResolverInterface
{
public:
  /**
   * Constructor.
   */
  DependencyResolverInterface() {}

  /**
   * Return a set containing the names of items requested by the object.
   */
  virtual const std::set<std::string> & getRequestedItems() = 0;

  /**
   * Return a set containing the names of items owned by the object.
   */
  virtual const std::set<std::string> & getSuppliedItems() = 0;

  /**
   * Given a vector, sort using the getRequested/SuppliedItems sets.
   */
  template <typename T>
  static void sort(typename std::vector<T> & vector);

  /**
   * Given a vector, sort using the depth-first search
   */
  template <typename T>
  static void sortDFS(typename std::vector<T> & vector);

  /**
   * A helper method for cyclic errors.
   */
  template <typename T, typename T2>
  static void cyclicDependencyError(CyclicDependencyException<T2> & e, const std::string & header);
};

template <typename T>
void
DependencyResolverInterface::sort(typename std::vector<T> & vector)
{
  sortDFS(vector);
}

template <typename T>
void
DependencyResolverInterface::sortDFS(typename std::vector<T> & vector)
{
  if (vector.size() <= 1)
    return;

  /**
   * Class that represents the dependency as a graph
   */
  DependencyResolver<T> graph;

  // Map of suppliers: what is supplied -> by what object
  std::multimap<std::string, T> suppliers_map;
  for (auto & v : vector)
  {
    for (const auto & supplied_item : v->getSuppliedItems())
    {
      suppliers_map.emplace(supplied_item, v);
      graph.addNode(v);
    }
  }

  // build the dependency graph
  for (auto & v : vector)
  {
    for (const auto & requested_item : v->getRequestedItems())
    {
      const auto & [begin_it, end_it] = suppliers_map.equal_range(requested_item);
      if (begin_it == end_it)
        graph.addNode(v);
      else
        for (const auto & [supplier_name, supplier_object] : as_range(begin_it, end_it))
        {
          libmesh_ignore(supplier_name);
          if (supplier_object == v)
            // We allow an object to have a circular dependency within itself; e.g. we choose to
            // trust a developer knows what they are doing within a single object
            continue;
          graph.addEdge(supplier_object, v);
        }
    }
  }

  vector = graph.dfs();
}

template <typename T, typename T2>
void
DependencyResolverInterface::cyclicDependencyError(CyclicDependencyException<T2> & e,
                                                   const std::string & header)
{
  std::ostringstream oss;

  oss << header << ":\n";
  const auto cycle = e.getCyclicDependencies();
  std::vector<std::string> names(cycle.size());
  for (const auto i : index_range(cycle))
    names[i] = static_cast<T>(cycle[i])->name();
  oss << MooseUtils::join(names, " <- ");
  mooseError(oss.str());
}
