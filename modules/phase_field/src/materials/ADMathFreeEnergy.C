//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMathFreeEnergy.h"

registerADMooseObject("PhaseFieldApp", ADMathFreeEnergy);

defineADValidParams(
    ADMathFreeEnergy,
    ADMaterial,
    params.addClassDescription("Material that implements the math free energy and its derivatives: "
                               "\nF = 1/4(1 + c)^2*(1 - c)^2");
    params.addParam<MaterialPropertyName>("f_name", "F", "function property name");
    params.addRequiredCoupledVar("c", "Concentration variable"););

template <ComputeStage compute_stage>
ADMathFreeEnergy<compute_stage>::ADMathFreeEnergy(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _c(adCoupledValue("c")),
    _f_name(adGetParam<MaterialPropertyName>("f_name")),
    _prop_F(adDeclareADProperty<Real>(_f_name)),
    _prop_dFdc(adDeclareADProperty<Real>(propertyNameFirst(_f_name, this->getVar("c", 0)->name())))
{
}

template <ComputeStage compute_stage>
void
ADMathFreeEnergy<compute_stage>::computeQpProperties()
{
  _prop_F[_qp] = 1.0 / 4.0 * (1.0 + _c[_qp]) * (1.0 + _c[_qp]) * (1.0 - _c[_qp]) * (1.0 - _c[_qp]);
  _prop_dFdc[_qp] = _c[_qp] * (_c[_qp] * _c[_qp] - 1.0);
}
