//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForceNodalKernel.h"

registerMooseObject("MooseApp", CoupledForceNodalKernel);
registerMooseObject("MooseApp", ADCoupledForceNodalKernel);

template <bool is_ad>
InputParameters
CoupledForceNodalKernelTempl<is_ad>::validParams()
{
  InputParameters params = GenericNodalKernel<is_ad>::validParams();
  params.addClassDescription("Adds a force proportional to the value of the coupled variable");
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");

  return params;
}

template <bool is_ad>
CoupledForceNodalKernelTempl<is_ad>::CoupledForceNodalKernelTempl(
    const InputParameters & parameters)
  : GenericNodalKernel<is_ad>(parameters),
    _v_var(coupled("v")),
    _v(this->template coupledGenericValue<is_ad>("v")),
    _coef(this->template getParam<Real>("coef"))
{
  if (_var.number() == _v_var)
    this->paramError("v",
                     "Coupled variable 'v' needs to be different from 'variable' with "
                     "CoupledForceNodalKernel / ADCoupledForceNodalKernel, consider using "
                     "ReactionNodalKernel / ADReactionNodalKernel or something similar");
}

template <bool is_ad>
GenericReal<is_ad>
CoupledForceNodalKernelTempl<is_ad>::computeQpResidual()
{
  return -_coef * _v[_qp];
}

template <bool is_ad>
Real
CoupledForceNodalKernelTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  mooseAssert(!is_ad,
              "In ADCoupledForceNodalKernel, computeQpOffDiagJacobian should not be called. Check "
              "computeJacobian implementation.");
  if (jvar == _v_var)
    return -_coef;
  return 0.0;
}

template class CoupledForceNodalKernelTempl<false>;
template class CoupledForceNodalKernelTempl<true>;
