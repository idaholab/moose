//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearElasticTruss.h"

registerMooseObject("TensorMechanicsApp", LinearElasticTruss);

InputParameters
LinearElasticTruss::validParams()
{
  InputParameters params = TrussMaterial::validParams();
  params.addClassDescription("Computes the linear elastic strain for a truss element");
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
