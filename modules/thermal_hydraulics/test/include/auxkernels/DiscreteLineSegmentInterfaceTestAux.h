//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "DiscreteLineSegmentInterface.h"

/**
 * Tests DiscreteLineSegmentInterface
 */
class DiscreteLineSegmentInterfaceTestAux : public AuxKernel, public DiscreteLineSegmentInterface
{
public:
  DiscreteLineSegmentInterfaceTestAux(const InputParameters & params);

protected:
  virtual Real computeValue() override;

  /// Which interface to test
  const MooseEnum & _test_type;

public:
  static InputParameters validParams();
};
