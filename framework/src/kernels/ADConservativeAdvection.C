//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConservativeAdvection.h"

registerMooseObject("MooseApp", ADConservativeAdvection);

InputParameters
ADConservativeAdvection::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Conservative form of $\\nabla \\cdot \\vec{v} u$ which in its weak "
                             "form is given by: $(-\\nabla \\psi_i, \\vec{v} u)$.");
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addParam<MaterialPropertyName>("advected_quantity",
                                        "An optional material property to be advected. If not "
                                        "supplied, then the variable will be used.");
  return params;
}

ADConservativeAdvection::ADConservativeAdvection(const InputParameters & parameters)
  : ADKernel(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _adv_quant(isParamValid("advected_quantity")
                   ? getADMaterialProperty<Real>("advected_quantity").get()
                   : _u)
{
}

ADReal
ADConservativeAdvection::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _velocity[_qp] * _adv_quant[_qp];
}
