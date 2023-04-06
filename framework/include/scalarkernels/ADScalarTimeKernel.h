//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"

class ADScalarTimeKernel : public ADScalarKernel
{
public:
  static InputParameters validParams();

  ADScalarTimeKernel(const InputParameters & parameters);

protected:
  /// Time derivative of the kernel variable
  const ADVariableValue & _u_dot;
};
