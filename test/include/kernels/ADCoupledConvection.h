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

/**
 * Define the ADKernel for a convection operator that looks like:
 *
 * grad_some_var dot u'
 */
class ADCoupledConvection : public ADKernel
{
public:
  static InputParameters validParams();

  ADCoupledConvection(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const ADVariableGradient & _velocity_vector;

  const Real & _scale;
};
