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
 * Applies a residual equal to
 * \f$\epsilon \frac{\partial u}{\partial t}\f$
 */
class FVPorosityTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  FVPorosityTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// The porosity
  const MaterialProperty<Real> & _eps;
};
