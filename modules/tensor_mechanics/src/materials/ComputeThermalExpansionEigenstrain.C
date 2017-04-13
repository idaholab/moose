/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeThermalExpansionEigenstrain.h"

template <>
InputParameters
validParams<ComputeThermalExpansionEigenstrain>()
{
  InputParameters params = validParams<ComputeThermalExpansionEigenstrainBase>();
  params.addClassDescription(
      "Computes eigenstrain due to thermal expansion with a constant coefficient");
  params.addParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");

  return params;
}

ComputeThermalExpansionEigenstrain::ComputeThermalExpansionEigenstrain(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBase(parameters),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

void
ComputeThermalExpansionEigenstrain::computeThermalStrain(Real & thermal_strain,
                                                         Real & instantaneous_cte)
{
  thermal_strain = _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature);
  instantaneous_cte = _thermal_expansion_coeff;
}
