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

#include <string>
#include <set>
#include <iostream>
#include <algorithm>

#include "DependencyResolver.h"

class DependencyResolverInterface
{
public:
  DependencyResolverInterface() {}

  virtual const std::set<std::string> & getRequestedItems() = 0;
  virtual const std::set<std::string> & getSuppliedItems() = 0;

  template<typename Iter>
  static
  void sort(Iter start, Iter end)
    {
      DependencyResolver<DependencyResolverInterface *> resolver;

      for (Iter iter = start; iter != end ; ++iter)
      {
        const std::set<std::string> & requested_items = (*iter)->getRequestedItems();

        for (Iter iter2 = start; iter2 != end; ++iter2)
        {
          if (iter == iter2) continue;
          const std::set<std::string> & supplied_items = (*iter2)->getSuppliedItems();


          std::set<std::string> intersect;
          std::set_intersection(requested_items.begin(), requested_items.end(), supplied_items.begin(),
                                supplied_items.end(), std::inserter(intersect, intersect.end()));

          // If the intersection isn't empty then there is a dependency here
          if (!intersect.empty())
            resolver.insertDependency(*iter, *iter2);
        }
      }

      // Sort based on dependencies
      std::sort(start, end, resolver);
    }

};

#endif // DEPENDENCYRESOLVERINTERFACE_H
