/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeThermalExpansionEigenstrain.h"
#include "RankTwoTensor.h"

template<>
InputParameters validParams<ComputeThermalExpansionEigenstrain>()
{
  InputParameters params = validParams<ComputeEigenstrainBase>();
  params.addClassDescription("Computes Eigenstrain due to thermal expansion");
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addParam<Real>("thermal_expansion_coeff", 0.0, "Thermal expansion coefficient");
  params.addDeprecatedParam<Real>("stress_free_reference_temperature", 273, "Reference temperature for thermal eigenstrain calculation", "'stress_free_temperature' has replaced this parameter");
  params.addParam<Real>("stress_free_temperature", "Reference temperature for thermal eigenstrain calculation");

  return params;
}

ComputeThermalExpansionEigenstrain::ComputeThermalExpansionEigenstrain(const InputParameters & parameters) :
    ComputeEigenstrainBase(parameters),
    _temperature(coupledValue("temperature")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _thermal_expansion_tensor(declareProperty<RankTwoTensor>(_base_name + "_thermal_expansion_tensor"))
{
  if (isParamValid("stress_free_temperature"))
    _stress_free_temperature = getParam<Real>("stress_free_temperature");
  else if (isParamValid("stress_free_reference_temperature"))
    _stress_free_temperature = getParam<Real>("stress_free_reference_temperature");
  else
    mooseError("Please specify 'stress_free_temperature'.");
}

void
ComputeThermalExpansionEigenstrain::computeQpEigenstrain()
{
  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(_thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature));

  _thermal_expansion_tensor[_qp].zero();
  _thermal_expansion_tensor[_qp].addIa(-_thermal_expansion_coeff);
}
