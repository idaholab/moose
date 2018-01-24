//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaskedBodyForce.h"
#include "Function.h"

template <>
InputParameters
validParams<MaskedBodyForce>()
{
  InputParameters params = validParams<BodyForce>();
  params.addClassDescription("Kernel that defines a body force modified by a material mask");
  params.addParam<MaterialPropertyName>("mask", "Material property defining the mask");
  return params;
}

MaskedBodyForce::MaskedBodyForce(const InputParameters & parameters)
  : BodyForce(parameters), _mask(getMaterialProperty<Real>("mask"))
{
}

Real
MaskedBodyForce::computeQpResidual()
{
  return BodyForce::computeQpResidual() * _mask[_qp];
}
