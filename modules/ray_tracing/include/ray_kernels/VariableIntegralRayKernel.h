//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegralRayKernel.h"
#include "MooseVariableInterface.h"

class VariableIntegralRayKernel : public IntegralRayKernel
{
public:
  VariableIntegralRayKernel(const InputParameters & params);

  static InputParameters validParams();

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
};
