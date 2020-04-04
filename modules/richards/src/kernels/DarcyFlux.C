//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyFlux.h"
#include "Material.h"

registerMooseObject("RichardsApp", DarcyFlux);

InputParameters
DarcyFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<RealVectorValue>(
      "fluid_weight",
      "Fluid weight (gravity*density) as a vector pointing downwards (usually "
      "measured in kg.m^-2.s^-2 = Pa/m).  Eg '0 0 -10000'");
  params.addRequiredParam<Real>("fluid_viscosity",
                                "Fluid dynamic viscosity (usually measured in Pa.s)");
  params.addClassDescription("Darcy flux.  nabla_i (k_ij/mu (nabla_j P - w_j)), where k_ij is the "
                             "permeability tensor, mu is the fluid viscosity, P is the fluid "
                             "pressure, and w_j is the fluid weight");
  return params;
}

DarcyFlux::DarcyFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _fluid_weight(getParam<RealVectorValue>("fluid_weight")),
    _fluid_viscosity(getParam<Real>("fluid_viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability"))
{
}

Real
DarcyFlux::computeQpResidual()
{
  return _grad_test[_i][_qp] * (_permeability[_qp] * (_grad_u[_qp] - _fluid_weight)) /
         _fluid_viscosity;
}

Real
DarcyFlux::computeQpJacobian()
{
  return _grad_test[_i][_qp] * (_permeability[_qp] * _grad_phi[_j][_qp]) / _fluid_viscosity;
}
