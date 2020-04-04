//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LaplacianSplit.h"

registerMooseObject("PhaseFieldApp", LaplacianSplit);

InputParameters
LaplacianSplit::validParams()
{
  InputParameters params = KernelGrad::validParams();
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
