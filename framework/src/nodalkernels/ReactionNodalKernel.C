//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactionNodalKernel.h"

#include "Function.h"

registerMooseObject("MooseApp", ReactionNodalKernel);
registerMooseObject("MooseApp", ADReactionNodalKernel);

template <bool is_ad>
InputParameters
ReactionNodalKernelTempl<is_ad>::validParams()
{
  InputParameters params = GenericNodalKernel<is_ad>::validParams();
  params.addClassDescription("Implements a simple consuming reaction term at nodes");
  params.addParam<Real>("coeff", 1., "A coefficient for multiplying the reaction term");
  return params;
}

template <bool is_ad>
ReactionNodalKernelTempl<is_ad>::ReactionNodalKernelTempl(const InputParameters & parameters)
  : GenericNodalKernel<is_ad>(parameters), _coeff(this->template getParam<Real>("coeff"))
{
}

template <bool is_ad>
GenericReal<is_ad>
ReactionNodalKernelTempl<is_ad>::computeQpResidual()
{
  return _coeff * _u[_qp];
}

template <bool is_ad>
Real
ReactionNodalKernelTempl<is_ad>::computeQpJacobian()
{
  mooseAssert(!is_ad,
              "In ADReactionNodalKernel, computeQpJacobian should not be called. Check "
              "computeJacobian implementation.");
  return _coeff;
}

template class ReactionNodalKernelTempl<false>;
template class ReactionNodalKernelTempl<true>;
