//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Gravity.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", Gravity);
registerMooseObject("TensorMechanicsApp", ADGravity);

template <bool is_ad>
InputParameters
GravityTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Apply gravity. Value is in units of acceleration.");
  params.addParam<bool>("use_displaced_mesh", true, "Displaced mesh defaults to true");
  params.addRequiredParam<Real>(
      "value", "Value multiplied against the residual, e.g. gravitational acceleration");
  params.addParam<FunctionName>(
      "function", "1", "A function that describes the gravitational force");
  params.addParam<Real>("alpha", 0.0, "alpha parameter required for HHT time integration scheme");
  params.addParam<MaterialPropertyName>("density", "density", "The density");
  return params;
}

template <bool is_ad>
GravityTempl<is_ad>::GravityTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _density(this->template getGenericMaterialProperty<Real, is_ad>("density")),
    _value(this->template getParam<Real>("value")),
    _function(this->getFunction("function")),
    _alpha(this->template getParam<Real>("alpha"))
{
}

template <bool is_ad>
GenericReal<is_ad>
GravityTempl<is_ad>::computeQpResidual()
{
  Real factor = _value * _function.value(_t + _alpha * _dt, _q_point[_qp]);
  return _density[_qp] * _test[_i][_qp] * -factor;
}

template class GravityTempl<false>;
template class GravityTempl<true>;
