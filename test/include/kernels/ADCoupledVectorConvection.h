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

class ADCoupledVectorConvection : public ADKernel
{
public:
  static InputParameters validParams();

  ADCoupledVectorConvection(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const bool & _use_grad;
  const ADVectorVariableValue & _velocity_vector;
  const ADVectorVariableGradient & _grad_velocity_vector;
};
