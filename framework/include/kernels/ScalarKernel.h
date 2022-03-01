//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernelBase.h"

class ScalarKernel : public ScalarKernelBase
{
public:
  static InputParameters validParams();

  ScalarKernel(const InputParameters & parameters);

protected:
  /// The current solution (old solution if explicit)
  const VariableValue & _u;
};
