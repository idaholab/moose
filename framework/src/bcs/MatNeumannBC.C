//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatNeumannBC.h"

registerMooseObject("MooseApp", MatNeumannBC);

InputParameters
MatNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addParam<Real>("value", 1.0, "The value to be enforced on the boundary.");
  params.declareControllable("value");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{C \\partial u}{\\partial n}=M*h$, "
                             "where $h$ is a constant, $M$ is a material property, and $C$ is a "
                             "coefficient defined by the kernel for $u$.");
  params.addRequiredParam<MaterialPropertyName>(
      "boundary_material",
      "Material property multiplying the constant that will be enforced by the BC");
  return params;
}

MatNeumannBC::MatNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _value(getParam<Real>("value")),
    _boundary_prop(getMaterialProperty<Real>("boundary_material"))
{
}

Real
MatNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _value * _boundary_prop[_qp];
}
