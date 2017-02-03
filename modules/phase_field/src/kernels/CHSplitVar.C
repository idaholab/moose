/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CHSplitVar.h"

template<>
InputParameters validParams<CHSplitVar>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Alternative Cahn-Hilliard split");
  params.addRequiredCoupledVar("c", "Variable representing the laplacian of c");
  return params;
}

CHSplitVar::CHSplitVar(const InputParameters & parameters) :
    KernelGrad(parameters),
    _var_c(coupled("c")),
    _grad_c(coupledGradient("c"))
{
}

RealGradient
CHSplitVar::precomputeQpResidual()
{
  return  _grad_c[_qp]; // * _grad_test[_i][_qp]
}

RealGradient
CHSplitVar::precomputeQpJacobian()
{
  return 0.0;
}

Real
CHSplitVar::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var_c)
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp];

  return 0.0;
}
