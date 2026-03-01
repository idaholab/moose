//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayReactionNodalKernel.h"

registerMooseObject("MooseApp", ArrayReactionNodalKernel);
registerMooseObject("MooseApp", ADArrayReactionNodalKernel);

template <bool is_ad>
InputParameters
ArrayReactionNodalKernelTempl<is_ad>::validParams()
{
  InputParameters params = GenericArrayNodalKernel<is_ad>::validParams();
  params.addClassDescription(
      "Implements a simple consuming reaction term at nodes for an array variable");
  params.addRequiredParam<RealEigenVector>("coeff",
                                           "Coefficients for multiplying the reaction term");
  return params;
}

template <bool is_ad>
ArrayReactionNodalKernelTempl<is_ad>::ArrayReactionNodalKernelTempl(
    const InputParameters & parameters)
  : GenericArrayNodalKernel<is_ad>(parameters),
    _coeff(this->template getParam<RealEigenVector>("coeff"))
{
  if (_coeff.size() != _count)
    paramError("coeff",
               "The size of the coefficient vector must match the size of the array variable");
}

template <bool is_ad>
void
ArrayReactionNodalKernelTempl<is_ad>::computeQpResidual(GenericRealEigenVector<is_ad> & residual)
{
  residual = _coeff.cwiseProduct(_u[_qp]);
}

template <bool is_ad>
void
ArrayReactionNodalKernelTempl<is_ad>::computeQpJacobian()
{
  for (const auto i : index_range(_coeff))
    setJacobian(i, i, _coeff(i));
}

template class ArrayReactionNodalKernelTempl<false>;
template class ArrayReactionNodalKernelTempl<true>;
