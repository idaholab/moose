//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TWORMTESTER_H
#define TWORMTESTER_H

#include "ElemSideNeighborLayersTester.h"

// Forward Declarations
class TwoRMTester;

template <>
InputParameters validParams<TwoRMTester>();

/**
 * Tests that the same RM can be used twice with the same object
 */
class TwoRMTester : public ElemSideNeighborLayersTester
{
public:
  TwoRMTester(const InputParameters & parameters);
};

#endif // TWORMTESTER_H
