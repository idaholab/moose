//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// CPPUnit includes
#include "gtest_include.h"

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
    _strict_ordering.addNode("a");
    _strict_ordering.addNode("b");
    _strict_ordering.addNode("c");
    _strict_ordering.addNode("d");
    _strict_ordering.addNode("e");
    _strict_ordering.addNode("f");
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
