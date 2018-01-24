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

#ifndef DEPENDENCYRESOLVERINTERFACE_H
#define DEPENDENCYRESOLVERINTERFACE_H

// STL includes
#include <string>
#include <set>
#include <iostream>
#include <algorithm>

// MOOSE includes
#include "DependencyResolver.h"

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
   * A helper method for cyclic errors.
   */
  template <typename T>
  static void cyclicDependencyError(CyclicDependencyException<T> & e, const std::string & header);
};

template <typename T>
void
DependencyResolverInterface::sort(typename std::vector<T> & vector)
{
  DependencyResolver<T> resolver;

  typename std::vector<T>::iterator start = vector.begin();
  typename std::vector<T>::iterator end = vector.end();

  for (typename std::vector<T>::iterator iter = start; iter != end; ++iter)
  {
    const std::set<std::string> & requested_items = (*iter)->getRequestedItems();

    for (typename std::vector<T>::iterator iter2 = start; iter2 != end; ++iter2)
    {
      if (iter == iter2)
        continue;

      const std::set<std::string> & supplied_items = (*iter2)->getSuppliedItems();

      std::set<std::string> intersect;
      std::set_intersection(requested_items.begin(),
                            requested_items.end(),
                            supplied_items.begin(),
                            supplied_items.end(),
                            std::inserter(intersect, intersect.end()));

      // If the intersection isn't empty then there is a dependency here
      if (!intersect.empty())
        resolver.insertDependency(*iter, *iter2);
    }
  }

  // Sort based on dependencies
  std::stable_sort(start, end, resolver);
}

template <typename T>
void
DependencyResolverInterface::cyclicDependencyError(CyclicDependencyException<T> & e,
                                                   const std::string & header)
{
  std::ostringstream oss;

  oss << header << ":\n";
  const typename std::multimap<T, T> & depends = e.getCyclicDependencies();
  for (typename std::multimap<T, T>::const_iterator it = depends.begin(); it != depends.end(); ++it)
    oss << (static_cast<T>(it->first))->name() << " -> " << (static_cast<T>(it->second))->name()
        << "\n";
  mooseError(oss.str());
}

#endif // DEPENDENCYRESOLVERINTERFACE_H
