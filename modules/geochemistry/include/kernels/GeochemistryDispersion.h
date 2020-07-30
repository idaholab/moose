//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AnisotropicDiffusion.h"

/**
 * Kernel describing grad(porosity * dispersion * grad(concentration)), where porosity is an
 * AuxVariable, dispersion is the hydrodynamic dispersion tensor (input by user as tensor_coeff),
 * and concentration is the variable for this Kernel
 */
class GeochemistryDispersion : public AnisotropicDiffusion
{
public:
  static InputParameters validParams();

  GeochemistryDispersion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  const VariableValue & _porosity;
};
