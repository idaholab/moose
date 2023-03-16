//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMathFreeEnergy.h"

registerMooseObject("PhaseFieldApp", ADMathFreeEnergy);

InputParameters
ADMathFreeEnergy::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Material that implements the math free energy and its derivatives: "
                             "\n$F = 1/4(1 + c)^2(1 - c)^2$");
  params.addParam<MaterialPropertyName>("f_name", "F", "function property name");
  params.addRequiredCoupledVar("c", "Concentration variable");
  return params;
}

ADMathFreeEnergy::ADMathFreeEnergy(const InputParameters & parameters)
  : Material(parameters),
    _c(adCoupledValue("c")),
    _f_name(getParam<MaterialPropertyName>("f_name")),
    _prop_F(declareADProperty<Real>(_f_name)),
    _prop_dFdc(
        declareADProperty<Real>(derivativePropertyNameFirst(_f_name, this->coupledName("c", 0))))
{
}

void
ADMathFreeEnergy::computeQpProperties()
{
  _prop_F[_qp] = 1.0 / 4.0 * (1.0 + _c[_qp]) * (1.0 + _c[_qp]) * (1.0 - _c[_qp]) * (1.0 - _c[_qp]);
  _prop_dFdc[_qp] = _c[_qp] * (_c[_qp] * _c[_qp] - 1.0);
}
