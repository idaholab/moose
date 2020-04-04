//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechImplicitEuler.h"
#include "SubProblem.h"

registerMooseObjectDeprecated("SolidMechanicsApp", SolidMechImplicitEuler, "07/30/2020 24:00");

InputParameters
SolidMechImplicitEuler::validParams()
{
  InputParameters params = SecondDerivativeImplicitEuler::validParams();
  params.addParam<Real>("artificial_scaling", "Factor to replace rho/dt^2");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

SolidMechImplicitEuler::SolidMechImplicitEuler(const InputParameters & parameters)
  : SecondDerivativeImplicitEuler(parameters),
    _density(getMaterialProperty<Real>("density")),
    _artificial_scaling_set(parameters.isParamValid("artificial_scaling")),
    _artificial_scaling(_artificial_scaling_set ? getParam<Real>("artificial_scaling") : 1)
{
  mooseDeprecated(name(), ": SolidMechImplicitEuler is deprecated. \
                  The solid_mechanics module will be removed from MOOSE on July 31, 2020. \
                  Please update your input files to utilize the tensor_mechanics equivalents of \
                  models based on solid_mechanics. A detailed migration guide that was developed \
                  for BISON, but which is generally applicable to any MOOSE model is available at: \
                  https://mooseframework.org/bison/tutorials/mechanics_conversion/overview.html");
}

Real
SolidMechImplicitEuler::computeQpResidual()
{
  return scaling() * _density[_qp] * SecondDerivativeImplicitEuler::computeQpResidual();
}

Real
SolidMechImplicitEuler::computeQpJacobian()
{
  return scaling() * _density[_qp] * SecondDerivativeImplicitEuler::computeQpJacobian();
}

Real
SolidMechImplicitEuler::scaling()
{
  Real factor(_artificial_scaling);
  if (_artificial_scaling_set)
  {
    factor *= _dt * _dt / _density[_qp];
  }
  return factor;
}
