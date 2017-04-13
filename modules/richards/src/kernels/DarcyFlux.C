/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DarcyFlux.h"
#include "Material.h"

template <>
InputParameters
validParams<DarcyFlux>()
{
  InputParameters params = validParams<Kernel>();
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
