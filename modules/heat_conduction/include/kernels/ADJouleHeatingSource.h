//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * This kernel calculates the heat source term corresponding to joule heating,
 * Q = J * E = elec_cond * grad_phi * grad_phi, where phi is the electrical potential.
 */
class ADJouleHeatingSource : public ADKernelValue
{
public:
  static InputParameters validParams();

  ADJouleHeatingSource(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

private:
  const ADVariableGradient & _grad_elec;

  const ADMaterialProperty<Real> & _elec_cond;
};
