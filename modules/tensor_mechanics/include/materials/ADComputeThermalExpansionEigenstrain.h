//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeThermalExpansionEigenstrainBase.h"

/**
 * ADComputeThermalExpansionEigenstrain computes an eigenstrain for thermal expansion
 * with a constant expansion coefficient.
 */
class ADComputeThermalExpansionEigenstrain : public ADComputeThermalExpansionEigenstrainBase
{
public:
  static InputParameters validParams();

  ADComputeThermalExpansionEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(ADReal & thermal_strain) override;

  const Real & _thermal_expansion_coeff;

  using ADComputeThermalExpansionEigenstrainBase::computeThermalStrain;
};
