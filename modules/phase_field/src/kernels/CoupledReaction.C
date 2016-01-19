/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledReaction.h"

template<>
InputParameters validParams<CoupledReaction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("v", "Coupled variable add to the residual");
  return params;
}

CoupledReaction::CoupledReaction(const InputParameters & parameters) :
    Kernel(parameters),
    _v(coupledValue("v")),
    _v_var(coupled("v"))
{
}

Real
CoupledReaction::computeQpResidual()
{
  return _test[_i][_qp] * -_v[_qp];
}

Real
CoupledReaction::computeQpJacobian()
{
  return 0.0;
}

Real
CoupledReaction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _test[_i][_qp] * -_phi[_j][_qp];

  return 0.0;
}
