//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorMatDiffusion.h"

registerMooseObject("MooseTestApp", FunctorMatDiffusion);
registerMooseObject("MooseTestApp", ADFunctorMatDiffusion);

template <bool is_ad>
InputParameters
FunctorMatDiffusionTempl<is_ad>::validParams()
{
  auto params = ADKernel::validParams();
  params.template addParam<MooseFunctorName>("diffusivity", "D", "The diffusivity value");
  return params;
}

template <bool is_ad>
FunctorMatDiffusionTempl<is_ad>::FunctorMatDiffusionTempl(const InputParameters & parameters)
  : ADKernel(parameters), _diff(getFunctor<GenericReal<is_ad>>("diffusivity"))
{
}

template <bool is_ad>
ADReal
FunctorMatDiffusionTempl<is_ad>::computeQpResidual()
{
  const Moose::ElemQpArg r = {_current_elem, _qp, _qrule, _q_point[_qp]};
  return _diff(r, Moose::currentState()) * _grad_test[_i][_qp] *
         _var.gradient(r, Moose::currentState());
}

template class FunctorMatDiffusionTempl<false>;
template class FunctorMatDiffusionTempl<true>;
