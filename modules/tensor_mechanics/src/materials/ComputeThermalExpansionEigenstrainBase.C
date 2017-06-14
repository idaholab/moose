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
  params.addParam<Real>("stress_free_temperature",
                        "Reference temperature at which there is no "
                        "thermal expansion for thermal eigenstrain "
                        "calculation");

  return params;
}

ComputeThermalExpansionEigenstrainBase::ComputeThermalExpansionEigenstrainBase(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBase>(parameters),
    _temperature(coupledValue("temperature")),
    _deigenstrain_dT(declarePropertyDerivative<RankTwoTensor>(_eigenstrain_name,
                                                              getVar("temperature", 0)->name())),
    _stress_free_temperature(declareProperty<Real>("stress_free_temperature")),
    _stress_free_temperature_old(declarePropertyOld<Real>("stress_free_temperature"))
{
  if (isParamValid("stress_free_temperature"))
  {
    _stress_free_temperature_user = getParam<Real>("stress_free_temperature");
    _provided_stress_free_temperature = true;
  }
  else
  {
    _provided_stress_free_temperature = false;
  }
}

void
ComputeThermalExpansionEigenstrainBase::initQpStatefulProperties()
{
  if (_provided_stress_free_temperature)
  {
    _stress_free_temperature[_qp] = _stress_free_temperature_user;
  }
  else
  {
    _stress_free_temperature[_qp] = _temperature[_qp];
  }
}

void
ComputeThermalExpansionEigenstrainBase::computeQpEigenstrain()
{
  Real thermal_strain = 0.0;
  Real instantaneous_cte = 0.0;

  _stress_free_temperature[_qp] = _stress_free_temperature_old[_qp];
  computeThermalStrain(thermal_strain, instantaneous_cte);

  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(thermal_strain);

  _deigenstrain_dT[_qp].zero();
  _deigenstrain_dT[_qp].addIa(-instantaneous_cte);
}
