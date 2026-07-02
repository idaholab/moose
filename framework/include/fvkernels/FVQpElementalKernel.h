//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Base class for FV elemental kernels that need qp-indexed solution values.
 */
class FVQpElementalKernel : public FVElementalKernel
{
public:
  static InputParameters validParams();
  FVQpElementalKernel(const InputParameters & parameters);

protected:
  const ADVariableValue & _u;
};
