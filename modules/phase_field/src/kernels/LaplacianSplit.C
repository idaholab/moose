/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LaplacianSplit.h"

template <>
InputParameters
validParams<LaplacianSplit>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription(
      "Split with a variable that holds the Laplacian of a phase field variable.");
  params.addRequiredCoupledVar("c", "Field variable to take the Laplacian of");
  return params;
}

LaplacianSplit::LaplacianSplit(const InputParameters & parameters)
  : KernelGrad(parameters), _var_c(coupled("c")), _grad_c(coupledGradient("c"))
{
}

RealGradient
LaplacianSplit::precomputeQpResidual()
{
  return _grad_c[_qp]; // * _grad_test[_i][_qp]
}

RealGradient
LaplacianSplit::precomputeQpJacobian()
{
  return 0.0;
}

Real
LaplacianSplit::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var_c)
    return _grad_phi[_j][_qp] * _grad_test[_i][_qp];

  return 0.0;
}
