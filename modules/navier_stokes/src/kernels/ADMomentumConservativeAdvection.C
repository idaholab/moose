//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMomentumConservativeAdvection.h"

registerMooseObject("NavierStokesApp", ADMomentumConservativeAdvection);

InputParameters
ADMomentumConservativeAdvection::validParams()
{
  InputParameters params = ADVectorKernelGrad::validParams();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} \\vec{u}$ which in its weak "
                             "form is given by: $(-\\nabla \\psi_i, \\vec{v} \\vec{u})$.");
  params.addParam<MaterialPropertyName>(
      "rho_name", "rho", "The name of the density material property");
  return params;
}

ADMomentumConservativeAdvection::ADMomentumConservativeAdvection(const InputParameters & parameters)
  : ADVectorKernelGrad(parameters),
    _rho(getADMaterialProperty<Real>("rho_name"))
{
}

ADRealTensorValue
ADMomentumConservativeAdvection::precomputeQpResidual()
{
  auto vel = _u[_qp];
  ;
  ADRealTensorValue vv(_u[_qp](0)*vel(0), _u[_qp](0)*vel(1), _u[_qp](0)*vel(2),
    _u[_qp](1)*vel(0), _u[_qp](1)*vel(1), _u[_qp](1)*vel(2),
    _u[_qp](2)*vel(0), _u[_qp](2)*vel(1), _u[_qp](2)*vel(2));
  return - _rho[_qp] * vv;
}