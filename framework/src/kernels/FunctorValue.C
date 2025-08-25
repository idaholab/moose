//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorValue.h"
#include "FunctorInterface.h"

registerMooseObject("MooseApp", FunctorValue);
registerMooseObject("MooseApp", ADFunctorValue);

template <bool is_ad>
InputParameters
FunctorValueTempl<is_ad>::validParams()
{
  InputParameters params = KernelValueParent<is_ad>::validParams();
  params.addClassDescription(
      "Residual term (u - prop) to set variable u equal to a given functor.");
  params.addRequiredParam<MooseFunctorName>(
      "functor", "Functor to be used in the kernel");
  params.addParam<bool>(
      "positive", true, "If the kernel is positive, this is true, if negative, it is false");
  return params;
}

template <bool is_ad>
FunctorValueTempl<is_ad>::FunctorValueTempl(const InputParameters & parameters)
  : KernelValueParent<is_ad>(parameters),
    _kernel_sign(this->template getParam<bool>("positive") ? 1.0 : -1.0),
    _functor(&getFunctor<GenericReal<is_ad>>("functor"))
{
}

template <>
Real
FunctorValueTempl<false>::precomputeQpResidual()
{
  // what is the desired value at this QP?
  const Moose::ElemQpArg space_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  const auto dest_value = (_functor)(space_arg, Moose::currentState());

  // compute the residual
  return _kernel_sign * (dest_value - _u[_qp]);
}

template <>
ADReal
FunctorValueTempl<true>::precomputeQpResidual()
{
  // what is the desired value at this QP?
  const Moose::ElemQpArg space_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  const auto dest_value = (_functor)(space_arg, Moose::currentState());

  // compute the residual
  return _kernel_sign * (dest_value - _u[_qp]);
}

template <>
Real
FunctorValueTempl<false>::precomputeQpJacobian()
{
  return -_kernel_sign * _phi[_j][_qp];
}

template <>
Real
FunctorValueTempl<true>::precomputeQpJacobian()
{
  mooseError("This should never get called");
}

template class FunctorValueTempl<false>;
template class FunctorValueTempl<true>;
