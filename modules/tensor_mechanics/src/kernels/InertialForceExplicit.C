//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialForceExplicit.h"

registerMooseObject("TensorMechanicsApp", InertialForceExplicit);

template <>
InputParameters
validParams<InertialForceExplicit>()
{
  InputParameters params = validParams<SecondTimeDerivative>();
  params.addClassDescription("Calculates the residual for the interial force "
                             "($M \\cdot acceleration$)");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  return params;
}

InertialForceExplicit::InertialForceExplicit(const InputParameters & parameters)
  : SecondTimeDerivative(parameters), _density(getMaterialProperty<Real>("density"))
{
}

Real
InertialForceExplicit::computeQpResidual()
{
  return _density[_qp] * SecondTimeDerivative::computeQpResidual();
}

Real
InertialForceExplicit::computeQpJacobian()
{
  return _density[_qp] * SecondTimeDerivative::computeQpJacobian();
}
