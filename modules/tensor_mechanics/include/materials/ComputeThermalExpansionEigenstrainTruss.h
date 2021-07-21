//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeThermalExpansionEigenstrainTrussBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeThermalExpansionEigenstrainTruss computes an eigenstrain for thermal expansion
 * with a constant expansion coefficient.
 */
class ComputeThermalExpansionEigenstrainTruss : public ComputeThermalExpansionEigenstrainTrussBase
{
public:
  static InputParameters validParams();

  ComputeThermalExpansionEigenstrainTruss(const InputParameters & parameters);

protected:
  virtual void computeThermalStrain(Real & thermal_strain) override;

  /// Constant thermal expansion coefficient
  const Real & _thermal_expansion_coeff;
};
