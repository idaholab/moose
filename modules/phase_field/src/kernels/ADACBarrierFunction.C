//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACBarrierFunction.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", ADACBarrierFunction);

InputParameters
ADACBarrierFunction::validParams()
{
  InputParameters params = ADGrainGrowthBase::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "gamma", "The interface profile coefficient to use with the kernel");
  params.addClassDescription("Allen-Cahn kernel used when 'mu' is a function of variables");
  return params;
}

ADACBarrierFunction::ADACBarrierFunction(const InputParameters & parameters)
  : ADGrainGrowthBase(parameters),
    _gamma(getADMaterialProperty<Real>("gamma")),
    _dmudvar(getADMaterialProperty<Real>(this->derivativePropertyNameFirst("mu", _var.name())))
{
}

ADReal
ADACBarrierFunction::computeDFDOP()
{
  ADReal f0 = 0.25 + 0.25 * _u[_qp] * _u[_qp] * _u[_qp] * _u[_qp] - 0.5 * _u[_qp] * _u[_qp];
  ADReal sum_etaj = _u[_qp] * _u[_qp];

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    for (unsigned int j = i + 1; j < _op_num; ++j)
      sum_etaj += (*_vals[j])[_qp] * (*_vals[j])[_qp];

    f0 += 0.25 * Utility::pow<4>((*_vals[i])[_qp]) - 0.5 * Utility::pow<2>((*_vals[i])[_qp]);
    f0 += sum_etaj * Utility::pow<2>((*_vals[i])[_qp]) * _gamma[_qp];
  }
  return _dmudvar[_qp] * f0;
}
