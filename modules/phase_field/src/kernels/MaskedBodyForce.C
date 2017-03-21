/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
