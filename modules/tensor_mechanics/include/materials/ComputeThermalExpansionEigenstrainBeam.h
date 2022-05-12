//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeThermalExpansionEigenstrainBeamBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeThermalExpansionEigenstrainBeam computes an eigenstrain for thermal expansion
 * with a constant expansion coefficient.
 */
class ComputeThermalExpansionEigenstrainBeam : public ComputeThermalExpansionEigenstrainBeamBase
{
public:
  static InputParameters validParams();

  ComputeThermalExpansionEigenstrainBeam(const InputParameters & parameters);

protected:
  virtual Real computeThermalStrain() override;

  /// Constant thermal expansion coefficient
  const Real & _thermal_expansion_coeff;
};
