//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSoretCoeffTest.h"

registerMooseObject("MiscTestApp", ADSoretCoeffTest);

InputParameters
ADSoretCoeffTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("coupled_var", "A coupled variable");
  params.addRequiredCoupledVar("temperature", "The coupled temperature variable");
  return params;
}

ADSoretCoeffTest::ADSoretCoeffTest(const InputParameters & parameters)
  : Material(parameters),
    _coupled_var(adCoupledValue("coupled_var")),
    _temp(adCoupledValue("temperature")),
    _soret_coeff(declareADProperty<Real>("soret_coefficient"))
{
}

void
ADSoretCoeffTest::computeQpProperties()
{
  _soret_coeff[_qp] = _coupled_var[_qp] / _temp[_qp] / _temp[_qp];
}
