//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatBodyForce.h"

registerMooseObject("MooseApp", ADMatBodyForce);

InputParameters
ADMatBodyForce::validParams()
{
  InputParameters params = BodyForce::validParams();
  params.addClassDescription("Kernel that defines a body force modified by a material property");
  params.addRequiredParam<MaterialPropertyName>(
      "material_property", "Material property defining the property used for the body force");
  return params;
}

ADMatBodyForce::ADMatBodyForce(const InputParameters & parameters)
  : ADBodyForce(parameters), _property(getADMaterialProperty<Real>("material_property"))
{
}

ADReal
ADMatBodyForce::computeQpResidual()
{
  return ADBodyForce::computeQpResidual() * _property[_qp];
}
