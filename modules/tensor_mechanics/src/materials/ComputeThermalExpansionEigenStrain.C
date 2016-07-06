/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeThermalExpansionEigenStrain.h"

template<>
InputParameters validParams<ComputeThermalExpansionEigenStrain>()
{
  InputParameters params = validParams<ComputeStressFreeStrainBase>();
  params.addClassDescription("Computes Eigenstrain due to thermal expansion");
  params.addCoupledVar("temperature", 273, "Coupled temperature in units of Kelvin");
  params.addParam<Real>("thermal_expansion_coeff", 0.0, "Thermal expansion coefficient in 1/K");
  params.addParam<Real>("stress_free_reference_temperature", 273, "Reference temperature for thermal expansion in K; used in the first time step to calculate the temperature change");

  return params;
}

ComputeThermalExpansionEigenStrain::ComputeThermalExpansionEigenStrain(const InputParameters & parameters) :
    ComputeStressFreeStrainBase(parameters),
    _temperature(coupledValue("temperature")),
    _has_incremental_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _temperature_old(_has_incremental_strain ? & coupledValueOld("temperature") : NULL),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _stress_free_reference_temperature(getParam<Real>("stress_free_reference_temperature"))
{
}

void
ComputeThermalExpansionEigenStrain::computeQpStressFreeStrain()
{
  RankTwoTensor thermal_strain;
  thermal_strain.zero();
  Real old_temp = 0.0;

    if (!_has_incremental_strain || _t_step == 1) // total strain form always uses the ref temp
      old_temp = _stress_free_reference_temperature;

    if (_temperature_old)
      old_temp = (* _temperature_old)[_qp];

    thermal_strain.addIa(_thermal_expansion_coeff * (_temperature[_qp] - old_temp));

  _stress_free_strain[_qp] = thermal_strain;

  if (_incremental_form)  // Necessary to avoid zero values of incremental stress free strain with a linear temperature increase
    _stress_free_strain[_qp] += (*_stress_free_strain_old)[_qp];
}
