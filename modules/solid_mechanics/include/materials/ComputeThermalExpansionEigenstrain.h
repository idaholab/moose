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
 * ComputeThermalExpansionEigenstrain computes an eigenstrain for thermal expansion
 * with a constant expansion coefficient.
 */
template <bool is_ad>
class ComputeThermalExpansionEigenstrainTempl
  : public ComputeThermalExpansionEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeThermalExpansionEigenstrainTempl(const InputParameters & parameters);

protected:
  virtual ValueAndDerivative<is_ad> computeThermalStrain() override;

  const Real & _thermal_expansion_coeff;

  using ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::_qp;
  using ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::_temperature;
  using ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::_stress_free_temperature;
};

typedef ComputeThermalExpansionEigenstrainTempl<false> ComputeThermalExpansionEigenstrain;
typedef ComputeThermalExpansionEigenstrainTempl<true> ADComputeThermalExpansionEigenstrain;
