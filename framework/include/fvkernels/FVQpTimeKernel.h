//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

/**
 * Base class for FV time kernels that need qp-indexed solution values in addition to _u_dot.
 */
class FVQpTimeKernel : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVQpTimeKernel(const InputParameters & parameters);

protected:
  const ADVariableValue & _u;
};
