//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForce.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", CoupledForce);
registerMooseObject("MooseApp", ADCoupledForce);

template <bool is_ad>
InputParameters
CoupledForceTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();

  params.addClassDescription("Implements a source term proportional to the value of a coupled "
                             "variable. Weak form: $(\\psi_i, -\\sigma v)$.");
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");

  return params;
}

template <bool is_ad>
CoupledForceTempl<is_ad>::CoupledForceTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _v_var(coupled("v")),
    _v(this->template coupledGenericValue<is_ad>("v")),
    _coef(this->template getParam<Real>("coef"))
{
  if (_var.number() == _v_var)
    paramError("v",
               "Coupled variable 'v' needs to be different from 'variable' with CoupledForce / "
               "ADCoupledForce, consider using the CoefReaction kernel or something similar");
}

template <bool is_ad>
GenericReal<is_ad>
CoupledForceTempl<is_ad>::computeQpResidual()
{
  return -_coef * _v[_qp] * _test[_i][_qp];
}

template <bool is_ad>
Real
CoupledForceTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // This function will never be called for the AD version. But because C++ does
  // not support an optional function declaration based on a template parameter,
  // we must keep this template for all cases.
  mooseAssert(!is_ad,
              "In ADCoupledForce, computeQpJacobian should not be called. Check computeJacobian "
              "implementation.");
  if (jvar == _v_var)
    return -_coef * _phi[_j][_qp] * _test[_i][_qp];
  return 0.0;
}

template class CoupledForceTempl<false>;
template class CoupledForceTempl<true>;
