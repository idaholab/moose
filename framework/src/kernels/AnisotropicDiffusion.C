//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisotropicDiffusion.h"

registerMooseObject("MooseApp", AnisotropicDiffusion);

InputParameters
AnisotropicDiffusion::validParams()
{
  InputParameters p = Kernel::validParams();
  p.addClassDescription("Anisotropic diffusion kernel $\\nabla \\cdot -\\widetilde{k} \\nabla u$ "
                        "with weak form given by $(\\nabla \\psi_i, \\widetilde{k} \\nabla u)$.");
  p.addRequiredParam<RealTensorValue>("tensor_coeff",
                                      "The Tensor to multiply the Diffusion operator by");
  return p;
}

AnisotropicDiffusion::AnisotropicDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _k(getParam<RealTensorValue>("tensor_coeff"))
{
}

Real
AnisotropicDiffusion::computeQpResidual()
{
  return (_k * _grad_u[_qp]) * _grad_test[_i][_qp];
}

Real
AnisotropicDiffusion::computeQpJacobian()
{
  return (_k * _grad_phi[_j][_qp]) * _grad_test[_i][_qp];
}
