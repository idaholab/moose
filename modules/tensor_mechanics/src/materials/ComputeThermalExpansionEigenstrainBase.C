/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeThermalExpansionEigenstrainBase.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<ComputeThermalExpansionEigenstrainBase>()
{
  InputParameters params = validParams<ComputeEigenstrainBase>();
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addDeprecatedParam<Real>("stress_free_reference_temperature",
                                  "Reference temperature for thermal eigenstrain calculation",
                                  "'stress_free_temperature' has replaced this parameter");
  params.addParam<Real>("stress_free_temperature",
                        "Reference temperature for thermal eigenstrain calculation");

  return params;
}

ComputeThermalExpansionEigenstrainBase::ComputeThermalExpansionEigenstrainBase(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBase>(parameters),
    _temperature(coupledValue("temperature")),
    _deigenstrain_dT(declarePropertyDerivative<RankTwoTensor>(_eigenstrain_name,
                                                              getVar("temperature", 0)->name()))
{
  if (isParamValid("stress_free_temperature"))
    _stress_free_temperature = getParam<Real>("stress_free_temperature");
  else if (isParamValid("stress_free_reference_temperature"))
    _stress_free_temperature = getParam<Real>("stress_free_reference_temperature");
  else
    mooseError("Please specify 'stress_free_temperature'.");
}

void
ComputeThermalExpansionEigenstrainBase::computeQpEigenstrain()
{
  Real thermal_strain = 0.0;
  Real instantaneous_cte = 0.0;
  computeThermalStrain(thermal_strain, instantaneous_cte);

  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(thermal_strain);

  _deigenstrain_dT[_qp].zero();
  _deigenstrain_dT[_qp].addIa(-instantaneous_cte);
}
