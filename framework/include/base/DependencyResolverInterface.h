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

/// STL includes
#include <string>
#include <set>
#include <iostream>
#include <algorithm>

// MOOSE incluces
#include "InputParameters.h"
#include "DependencyResolver.h"
#include "MooseUtils.h"

class DependencyResolverInterface;

template<>
InputParameters validParams<DependencyResolverInterface>();

/**
 * A class for sorting objects based on inter dependencies
 */
class DependencyResolverInterface
{
public:
  DependencyResolverInterface(){}

  /**
   * Return a list of items being requested by an object
   */
  virtual const std::set<std::string> & getRequestedItems() = 0;

  /**
   * Return a list of items that are owned by an object
   */
  virtual const std::set<std::string> & getSuppliedItems() = 0;

  /**
   * Return a list of items that are iterable
   *
   * Iterable items are allowed to have cyclic dependencies, see the Material object
   * and test/tests/materials/recompute/recompute.i to see how this is utilized.
   */
  virtual const std::set<std::string> & getIterativeItems(){ return _empty_set; }

  template<typename Iter>
  static
  void sort(Iter start, Iter end, bool iterative_warning = true, bool debug = false)
  {
    DependencyResolver<DependencyResolverInterface *> resolver;

    // Loop over each item to be sorted
    for (Iter iter = start; iter != end ; ++iter)
    {

      // Extract the requested, supplied, and iterable items from the first object
      const std::set<std::string> & requested_items = (*iter)->getRequestedItems();
      const std::set<std::string> & supplied_items = (*iter)->getSuppliedItems();
      const std::set<std::string> & iterative_items = (*iter)->getIterativeItems(); // see Materail.h

      // Loop over the objects again to allow all objects to be compared against each other
      for (Iter iter2 = start; iter2 != end; ++iter2)
      {
        // Do not compare the object to itself
        if (iter == iter2) continue;

        // Extract the supplied and iterative items
        const std::set<std::string> & supplied_items2 = (*iter2)->getSuppliedItems();
        const std::set<std::string> & iterative_items2 = (*iter2)->getIterativeItems();

        // Compute the intersection of the supplied from object 2 and requested items of object 1
        std::set<std::string> intersect;
        std::set_intersection(requested_items.begin(), requested_items.end(), supplied_items2.begin(), supplied_items2.end(),
                              std::inserter(intersect, intersect.end()));

        if (debug)
        {
          const std::set<std::string> & requested_items2 = (*iter)->getRequestedItems();
          Moose::out << COLOR_BLUE << (*iter)->name() << COLOR_DEFAULT << " -> " << COLOR_RED << (*iter2)->name() << COLOR_DEFAULT << std::endl;
          Moose::out << COLOR_BLUE << (*iter)->name() << '\n';
          Moose::out << "  supplied:" << MooseUtils::dump(supplied_items) << '\n';
          Moose::out << " requested:" << MooseUtils::dump(requested_items) << '\n';
          Moose::out << " iterative:" << MooseUtils::dump(iterative_items) << "\n";
          Moose::out << COLOR_RED << (*iter2)->name() << '\n';
          Moose::out << "  supplied:" << MooseUtils::dump(supplied_items2) << '\n';
          Moose::out << " requested:" << MooseUtils::dump(requested_items2) << '\n';
          Moose::out << " iterative:" << MooseUtils::dump(iterative_items2) << COLOR_DEFAULT << "\n\n";
        }

        // If the intersection is NOT empty then there is a dependency between the objects
        if (!intersect.empty())
        {

          // Compute the intersection between the supplied from object 1 and the iterative items from object 2
          std::set<std::string> iterative_intersect;
          std::set_intersection(supplied_items.begin(), supplied_items.end(), iterative_items2.begin(), iterative_items2.end(),
                                std::inserter(iterative_intersect, iterative_intersect.begin()));

          // There is an allowable dependency if the intersection with iterative items contains values
          if (!iterative_intersect.empty())
          {

            // Compute difference between the supplied items and the items that are marked as iterable on the other object
            std::set<std::string> iterative_diff;
            std::set_difference(supplied_items.begin(), supplied_items.end(), iterative_items2.begin(), iterative_items2.end(),
                                std::inserter(iterative_diff, iterative_diff.begin()));

            // If difference is NOT empty then produce a warning that items may be recomputed when iterating
            if (iterative_warning && !iterative_diff.empty())
            {
              std::ostringstream oss;
              oss << "The following items are marked as iterable by '" << (*iter2)->name() << "' and owned by '" << (*iter)->name() << "':\n  ";
              for (std::set<std::string>::const_iterator it = iterative_intersect.begin(); it != iterative_intersect.end(); ++it)
                oss << *it << " ";
              oss << "\n\n";

              oss << "However, the '" << (*iter)->name() << "' also contains the following items not marked by '" << (*iter2)->name() << "' for iteration that may be recomputed when iterating:\n  ";
              for (std::set<std::string>::const_iterator it = iterative_diff.begin(); it != iterative_diff.end(); ++it)
                oss << *it << " ";
              oss << '\n';

              mooseDoOnce(mooseWarning(oss.str()));
            }
          }

          else
            resolver.insertDependency(*iter, *iter2);

        }
      }
    }

    // Sort based on dependencies
    std::stable_sort(start, end, resolver);
  }


private:

  /// getIterativeItems defaults to returning an empty set, this is the empty set
  std::set<std::string> _empty_set;
};

#endif // DEPENDENCYRESOLVERINTERFACE_H
