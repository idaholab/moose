/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LinearElasticTruss.h"

template <>
InputParameters
validParams<LinearElasticTruss>()
{
  InputParameters params = validParams<TrussMaterial>();
  params.addParam<Real>("thermal_expansion_coeff", 0.0, "Thermal expansion coefficient in 1/K");
  params.addParam<Real>("temperature_ref", 273, "Reference temperature for thermal expansion in K");
  params.addCoupledVar("temperature", 273, "Temperature in Kelvin");
  return params;
}

LinearElasticTruss::LinearElasticTruss(const InputParameters & parameters)
  : TrussMaterial(parameters),
    _T(coupledValue("temperature")),
    _T0(getParam<Real>("temperature_ref")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

void
LinearElasticTruss::computeQpStrain()
{
  _total_stretch[_qp] = _current_length / _origin_length - 1.0;
  _elastic_stretch[_qp] = _total_stretch[_qp] - _thermal_expansion_coeff * (_T[_qp] - _T0);
}

void
LinearElasticTruss::computeQpStress()
{
  _axial_stress[_qp] = _youngs_modulus[_qp] * _elastic_stretch[_qp];
}
