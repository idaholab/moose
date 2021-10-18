//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.template addParam<bool>(
      "use_preinitd_data",
      false,
      "Whether to do on the fly computation of variable data or assume that "
      "variable data has already been pre-initialized");
  return params;
}

template <bool is_ad>
FunctorMatDiffusionTempl<is_ad>::FunctorMatDiffusionTempl(const InputParameters & parameters)
  : ADKernel(parameters),
    _diff(getFunctor<GenericReal<is_ad>>("diffusivity")),
    _use_preinitd_data(getParam<bool>("use_preinitd_data"))
{
}

template <bool is_ad>
ADReal
FunctorMatDiffusionTempl<is_ad>::computeQpResidual()
{
  if (_use_preinitd_data)
    return _diff(std::make_tuple(Moose::ElementType::Element, _qp, _current_elem->subdomain_id())) *
           _grad_test[_i][_qp] * _var.gradient(std::make_tuple(_current_elem, _qp, _qrule));
  else
    return _diff(std::make_tuple(_current_elem, _qp, _qrule)) * _grad_test[_i][_qp] *
           _var.gradient(std::make_tuple(_current_elem, _qp, _qrule));
}

template class FunctorMatDiffusionTempl<false>;
template class FunctorMatDiffusionTempl<true>;
