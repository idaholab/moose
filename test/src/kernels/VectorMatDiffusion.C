//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMatDiffusion.h"

registerMooseObject("MooseTestApp", VectorMatDiffusion);

InputParameters
VectorMatDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<MaterialPropertyName>("coef",
                                        "The anisotropic (diagonal) vector diffusion coefficient");
  params.deprecateParam("coef", "coeff", "01/01/2040");
  params.addClassDescription("Diffusion kernel for a regular variable with anisotropic diffusion "
                             "coefficients as a vector.");
  return params;
}

VectorMatDiffusion::VectorMatDiffusion(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _coef(getMaterialProperty<RealVectorValue>("coef")),
    _grad_coef(getMaterialPropertyDerivative<RealVectorValue>("coef", _var.name()))
{
}

Real
VectorMatDiffusion::computeQpResidual()
{
  RealVectorValue grad_u(_coef[_qp](0) * _grad_u[_qp](0),
                         _coef[_qp](1) * _grad_u[_qp](1),
                         _coef[_qp](2) * _grad_u[_qp](2));
  return _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
VectorMatDiffusion::computeQpJacobian()
{
  RealVectorValue term_1(_coef[_qp](0) * _grad_phi[_j][_qp](0),
                         _coef[_qp](1) * _grad_phi[_j][_qp](1),
                         _coef[_qp](2) * _grad_phi[_j][_qp](2));
  RealVectorValue term_2(_grad_coef[_qp](0) * _phi[0][_qp],
                         _grad_coef[_qp](1) * _phi[1][_qp],
                         _grad_coef[_qp](2) * _phi[2][_qp]);

  return (term_1 + term_2) * _grad_test[_i][_qp];
}
