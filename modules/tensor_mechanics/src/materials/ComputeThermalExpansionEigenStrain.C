/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeThermalExpansionEigenStrain.h"
#include "RankTwoTensor.h"

template<>
InputParameters validParams<ComputeThermalExpansionEigenStrain>()
{
  InputParameters params = validParams<ComputeStressFreeStrainBase>();
  params.addClassDescription("Computes Eigenstrain due to thermal expansion");
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addParam<Real>("thermal_expansion_coeff", 0.0, "Thermal expansion coefficient");
  params.addDeprecatedParam<Real>("stress_free_reference_temperature", 273, "Reference temperature for thermal eigenstrain calculation", "'stress_free_temperature' has replaced this parameter");
  params.addParam<Real>("stress_free_temperature", "Reference temperature for thermal eigenstrain calculation");

  return params;
}

ComputeThermalExpansionEigenStrain::ComputeThermalExpansionEigenStrain(const InputParameters & parameters) :
    ComputeStressFreeStrainBase(parameters),
    _temperature(coupledValue("temperature")),
    _has_incremental_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _temperature_old(_has_incremental_strain ? & coupledValueOld("temperature") : NULL),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _thermal_expansion_tensor(declareProperty<RankTwoTensor>(_base_name + "_thermal_expansion_tensor")),
    _step_one(declareRestartableData<bool>("step_one", true))
{
  if (isParamValid("stress_free_temperature"))
    _stress_free_temperature = getParam<Real>("stress_free_temperature");
  else if (isParamValid("stress_free_reference_temperature"))
    _stress_free_temperature = getParam<Real>("stress_free_reference_temperature");
  else
    mooseError("Please specify 'stress_free_temperature'.");
}

void
ComputeThermalExpansionEigenStrain::computeQpStressFreeStrain()
{
  RankTwoTensor thermal_strain;
  thermal_strain.zero();
  Real old_temp = 0.0;
  if (_t_step >= 2)
    _step_one = false;

  if (!_has_incremental_strain || _step_one)
    old_temp = _stress_free_temperature;
  else
    old_temp = (* _temperature_old)[_qp];

  thermal_strain.addIa(_thermal_expansion_coeff * (_temperature[_qp] - old_temp));

  _stress_free_strain[_qp] = thermal_strain;

  if (_has_incremental_strain)  // Necessary to avoid zero values of incremental stress free strain with a linear temperature increase
    _stress_free_strain[_qp] += (*_stress_free_strain_old)[_qp];

  RankTwoTensor identity(RankTwoTensor::initIdentity);
  _thermal_expansion_tensor[_qp] = -_thermal_expansion_coeff * identity;
}
