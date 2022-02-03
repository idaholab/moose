//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

/**
 * This class is only currently used to test whether we can request neighbor AD calculations and not
 * have anything go horribly wrong
 */
class ADDGCoupledTest : public ADDGKernel
{
public:
  static InputParameters validParams();

  ADDGCoupledTest(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  MooseVariable & _v_var;
  const ADVariableValue & _v;
  const ADVariableValue & _v_neighbor;
};
