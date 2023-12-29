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
#include "DerivativeMaterialInterface.h"

/**
 * ComputeDilatationThermalExpansionEigenstrainBase computes an eigenstrain for thermal expansion
 * from an dilatation equation.
 */
template <bool is_ad>
class ComputeDilatationThermalExpansionEigenstrainBaseTempl
  : public ComputeThermalExpansionEigenstrainBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeDilatationThermalExpansionEigenstrainBaseTempl(const InputParameters & parameters);

protected:
  virtual ValueAndDerivative<is_ad> computeThermalStrain() override;

  /**
   * Compute the fractional linear dilatation due to thermal expansion delta L / L
   * along with its temperature derivative.
   * @param temperature current temperature
   * @return fractional linear dilatation with temperature derivative
   */
  virtual ValueAndDerivative<is_ad>
  computeDilatation(const ValueAndDerivative<is_ad> & temperature) = 0;

  using ComputeThermalExpansionEigenstrainBaseTempl<is_ad>::_qp;
};

typedef ComputeDilatationThermalExpansionEigenstrainBaseTempl<false>
    ComputeDilatationThermalExpansionEigenstrainBase;
typedef ComputeDilatationThermalExpansionEigenstrainBaseTempl<true>
    ADComputeDilatationThermalExpansionEigenstrainBase;
