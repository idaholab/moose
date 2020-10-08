//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericValueTest.h"

registerMooseObject("MooseTestApp", GenericValueTest);
registerMooseObject("MooseTestApp", ADGenericValueTest);

template <bool is_ad>
InputParameters
GenericValueTestTempl<is_ad>::validParams()
{
  return GenericKernel<is_ad>::validParams();
}

template <bool is_ad>
GenericValueTestTempl<is_ad>::GenericValueTestTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters)
{
}

template <bool is_ad>
GenericReal<is_ad>
GenericValueTestTempl<is_ad>::computeQpResidual()
{
  return -_u[_qp] * _test[_i][_qp];
}

template <bool is_ad>
Real
GenericValueTestTempl<is_ad>::computeQpJacobian()
{
  return -_phi[_j][_qp] * _test[_i][_qp];
}

template class GenericValueTestTempl<false>;
template class GenericValueTestTempl<true>;
