//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHeatMassTransfer.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowHeatMassTransfer);

template <>
InputParameters
validParams<PorousFlowHeatMassTransfer>()
{
  InputParameters params = validParams<Kernel>();

  params.addClassDescription("Calculate heat or mass transfer from a coupled variable to u");
  params.addRequiredCoupledVar(
      "v", "The variable which is tranfsered to u using a coefficient transfer_coefficient");
  params.addParam<Real>("transfer_coefficient",
                        1.0,
                        "Transfer coefficient for heat or mass transfer between variables");

  return params;
}

PorousFlowHeatMassTransfer::PorousFlowHeatMassTransfer(const InputParameters & parameters)
  : Kernel(parameters),
    _v_var(coupled("v")),
    _v(coupledValue("v")),
    _coef(getParam<Real>("transfer_coefficient"))
{
}

Real
PorousFlowHeatMassTransfer::computeQpResidual()
{
  return _coef * (_u[_qp] - _v[_qp]) * _test[_i][_qp];
}

Real
PorousFlowHeatMassTransfer::computeQpJacobian()
{
  return jac(_var.number());
}

Real
PorousFlowHeatMassTransfer::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

Real
PorousFlowHeatMassTransfer::jac(unsigned int jvar) const
{
  if (jvar == _var.number())
    return _coef * _phi[_j][_qp] * _test[_i][_qp];
  else if (jvar == _v_var)
    return -_coef * _phi[_j][_qp] * _test[_i][_qp];
  return 0.0;
}
