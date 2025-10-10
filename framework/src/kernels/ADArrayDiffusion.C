//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADArrayDiffusion.h"

registerMooseObject("MooseApp", ADArrayDiffusion);

InputParameters
ADArrayDiffusion::validParams()
{
  InputParameters params = ADArrayKernel::validParams();
  params += ADFunctorInterface::validParams();
  params.addClassDescription(
      "The array Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak "
      "form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

ADArrayDiffusion::ADArrayDiffusion(const InputParameters & parameters) : ADArrayKernel(parameters)
{
}

void
ADArrayDiffusion::computeQpResidual(ADRealEigenVector & residual)
{
  residual = _grad_u[_qp] * _array_grad_test[_i][_qp];
}
