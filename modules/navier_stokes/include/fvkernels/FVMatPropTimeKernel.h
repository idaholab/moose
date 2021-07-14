//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

/**
 * Applies a residual equal to a supplied material property which is supposed to represent a time
 * derivative, e.g. the derivative of the product of density and a passive scalar
 * \f$\frac{\partial \rho s}{\partial t}\f$
 */
class FVMatPropTimeKernel : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVMatPropTimeKernel(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _mat_prop_time_derivative;
};
