//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADLaplacianSplit.h"

registerMooseObject("PhaseFieldApp", ADLaplacianSplit);

InputParameters
ADLaplacianSplit::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription(
      "Split with a variable that holds the Laplacian of a phase field variable.");
  params.addRequiredCoupledVar("c", "Field variable to take the Laplacian of");
  return params;
}

ADLaplacianSplit::ADLaplacianSplit(const InputParameters & parameters)
  : ADKernelGrad(parameters), _var_c(adCoupledValue("c")), _grad_c(adCoupledGradient("c"))
{
}

ADRealGradient
ADLaplacianSplit::precomputeQpResidual()
{
  return _grad_c[_qp]; // * _grad_test[_i][_qp]
}
