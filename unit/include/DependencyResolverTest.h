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

#ifndef DEPENDENCYRESOLVERTEST_H
#define DEPENDENCYRESOLVERTEST_H

// CPPUnit includes
#include "gtest/gtest.h"

#include "DependencyResolver.h"

#include <string>

class DependencyResolverTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _resolver.insertDependency("b", "a");
    _resolver.insertDependency("c", "a");
    _resolver.insertDependency("d", "c");

    // There are no items in the same "set" so the ordering will be strict
    _strict_ordering.insertDependency("e", "b");
    _strict_ordering.insertDependency("a", "e");
    _strict_ordering.insertDependency("d", "a");
    // Ordering will be "bead"

    /**
     *          k0     k1
     *        / |  \  /
     *      m0  m1  m2
     *     / \   |\  |
     *    mA  mB | \ |
     *          \|  mD
     *           mC
     */
    _tree.insertDependency("k0", "m0");
    _tree.insertDependency("k0", "m1");
    _tree.insertDependency("k0", "m2");
    _tree.insertDependency("k1", "m2");
    _tree.insertDependency("m0", "mA");
    _tree.insertDependency("m0", "mB");
    _tree.insertDependency("m1", "mC");
    _tree.insertDependency("m1", "mD");
    _tree.insertDependency("mB", "mC");
    _tree.insertDependency("m1", "mD");
    _tree.insertDependency("m2", "mD");
  }

  DependencyResolver<std::string> _resolver;
  DependencyResolver<std::string> _strict_ordering;
  DependencyResolver<std::string> _tree;
};

#endif // DEPENDENCYRESOLVERTEST_H
