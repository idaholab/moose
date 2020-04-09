//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "SideIntegralPostprocessor.h"

/**
 * This postprocessor demonstrates coupling a scalar variable to a postprocessor
 */
class ScalarCoupledPostprocessor : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ScalarCoupledPostprocessor(const InputParameters & parameters);

protected:
  Real computeQpIntegral();

  const VariableValue & _coupled_scalar;
  const VariableValue & _u;
};
