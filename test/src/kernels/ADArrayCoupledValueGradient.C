//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayCoupledValueGradient.h"

registerMooseObject("MooseTestApp", ADArrayCoupledValueGradient);

InputParameters
ADArrayCoupledValueGradient::validParams()
{
  InputParameters params = ADArrayKernel::validParams();
  params.addCoupledVar("v", "The coupled array variable, or its constant component values");
  params.addRequiredParam<RealEigenVector>("coef", "Component-wise coefficient");
  params.addParam<RealVectorValue>(
      "direction", RealVectorValue(1, 0, 0), "Direction the gradient of v is projected on");
  params.addClassDescription("Adds the residual $-(\\vec{c} \\circ (\\vec{v} + \\nabla \\vec{v} "
                             "\\cdot \\vec{d}), \\vec{u}^\\ast)$ for a coupled array variable "
                             "$\\vec{v}$ using automatic differentiation.");
  return params;
}

ADArrayCoupledValueGradient::ADArrayCoupledValueGradient(const InputParameters & parameters)
  : ADArrayKernel(parameters),
    _v(adCoupledArrayValue("v")),
    _grad_v(adCoupledArrayGradient("v")),
    _coef(getParam<RealEigenVector>("coef")),
    _direction(getParam<RealVectorValue>("direction"))
{
  if (_coef.size() != _count)
    paramError("coef", "Must have as many entries as 'variable' has components");
  if (isCoupled("v") && getArrayVar("v", 0)->count() != _count)
    paramError("v", "Must have the same number of components as 'variable'");
}

void
ADArrayCoupledValueGradient::computeQpResidual(ADRealEigenVector & residual)
{
  ADRealEigenVector grad_v_dir = _grad_v[_qp].col(0) * _direction(0);
  for (const auto d : make_range(1u, unsigned(Moose::dim)))
    grad_v_dir += _grad_v[_qp].col(d) * _direction(d);

  residual = -_coef.cwiseProduct(_v[_qp] + grad_v_dir) * _test[_i][_qp];
}
