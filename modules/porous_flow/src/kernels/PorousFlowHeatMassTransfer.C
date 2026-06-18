//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHeatMassTransfer.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowHeatMassTransfer);
registerMooseObject("PorousFlowApp", ADPorousFlowHeatMassTransfer);

template <bool is_ad>
InputParameters
PorousFlowHeatMassTransferTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();

  params.addClassDescription(
      "Calculate heat or mass transfer from a coupled variable v to the variable u. "
      "No mass lumping is performed here.");
  params.addRequiredCoupledVar(
      "v", "The variable which is tranfsered to u using a transfer coefficient");
  params.addCoupledVar("transfer_coefficient",
                       1.0,
                       "Transfer coefficient for heat or mass transferred between variables");

  return params;
}

template <bool is_ad>
PorousFlowHeatMassTransferTempl<is_ad>::PorousFlowHeatMassTransferTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _v_var(coupled("v")),
    _v(this->template coupledGenericValue<is_ad>("v")),
    _coef_var(this->coupledValue("transfer_coefficient"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowHeatMassTransferTempl<is_ad>::computeQpResidual()
{
  return _coef_var[_qp] * (_u[_qp] - _v[_qp]) * _test[_i][_qp];
}

template <bool is_ad>
Real
PorousFlowHeatMassTransferTempl<is_ad>::computeQpJacobian()
{
  return jac(_var.number());
}

template <bool is_ad>
Real
PorousFlowHeatMassTransferTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

template <bool is_ad>
Real
PorousFlowHeatMassTransferTempl<is_ad>::jac(unsigned int jvar) const
{
  // Never called for the AD instantiation, which assembles the Jacobian from the AD residual.
  mooseAssert(!is_ad, "jac should not be called for the AD instantiation");
  if (jvar == _var.number())
    return _coef_var[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  else if (jvar == _v_var)
    return -_coef_var[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  return 0.0;
}

template class PorousFlowHeatMassTransferTempl<false>;
template class PorousFlowHeatMassTransferTempl<true>;
