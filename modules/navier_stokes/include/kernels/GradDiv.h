//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class GradDiv : public ADKernel
{
public:
  static InputParameters validParams();

  GradDiv(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableGradient & _grad_vel_x;
  const ADVariableGradient & _grad_vel_y;
  const unsigned short _comp;
  const bool _matrix_only;
};
