//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatchedValueBC.h"

registerMooseObject("MooseApp", MatchedValueBC);
registerMooseObject("MooseApp", ADMatchedValueBC);

template <bool is_ad>
InputParameters
MatchedValueBCTempl<is_ad>::validParams()
{
  InputParameters params = GenericNodalBC<is_ad>::validParams();
  params.addRequiredCoupledVar("v", "The variable whose value we are to match.");
  params.addParam<Real>("u_coeff", 1.0, " A coefficient for primary variable u");
  params.addParam<Real>("v_coeff", 1.0, " A coefficient for coupled variable v");
  params.addClassDescription("Implements a NodalBC which equates two different Variables' values "
                             "on a specified boundary.");
  return params;
}

template <bool is_ad>
MatchedValueBCTempl<is_ad>::MatchedValueBCTempl(const InputParameters & parameters)
  : GenericNodalBC<is_ad>(parameters),
    _v(this->template coupledGenericValue<is_ad>("v")),
    _v_num(coupled("v")),
    _u_coeff(this->template getParam<Real>("u_coeff")),
    _v_coeff(this->template getParam<Real>("v_coeff"))
{
}

template <>
GenericReal<true>
MatchedValueBCTempl<true>::computeQpResidual()
{
  return _u_coeff * _u - _v_coeff * _v[_qp];
}

template <>
GenericReal<false>
MatchedValueBCTempl<false>::computeQpResidual()
{
  return _u_coeff * _u[_qp] - _v_coeff * _v[_qp];
}

template <bool is_ad>
Real
MatchedValueBCTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // For the AD version, we do not need this implementation since AD will
  // automatically compute derivatives. In other words, this function will
  // never be called for the AD version. But we can not eliminate this function
  // for the AD because C++ does not support an optional function declaration based
  // on a template parameter.
  if (jvar == _v_num)
    return -_v_coeff;
  else
    return 0.;
}

template class MatchedValueBCTempl<false>;
template class MatchedValueBCTempl<true>;
