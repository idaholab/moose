/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "DarcyFluxComponent.h"

template <>
InputParameters
validParams<DarcyFluxComponent>()
{
  MooseEnum component("x=0 y=1 z=2");
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<RealVectorValue>(
      "fluid_weight",
      "Fluid weight (gravity*density) as a vector pointing downwards (usually "
      "measured in kg.m^-2.s^-2 = Pa/m).  Eg '0 0 -10000'");
  params.addRequiredParam<Real>("fluid_viscosity",
                                "Fluid dynamic viscosity (usually measured in Pa.s)");
  params.addClassDescription("Darcy flux (in m^3.s^-1.m^-2, or m.s^-1)  -(k_ij/mu (nabla_j P - "
                             "w_j)), where k_ij is the permeability tensor, mu is the fluid "
                             "viscosity, P is the fluid pressure, and w_j is the fluid weight.  If "
                             "velocity_scaling is used then -(k_ij/mu (nabla_j P - "
                             "w_j))/velocity_scaling is returned");
  params.addParam<MooseEnum>("component", component, "The component of the Darcy flux to return");
  params.addParam<Real>(
      "velocity_scaling",
      1,
      "Scale the result by (1/velocity_scaling).  Usually velocity_scaling = porosity.");
  params.addRequiredCoupledVar("porepressure", "The variable representing the porepressure");
  return params;
}

DarcyFluxComponent::DarcyFluxComponent(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_pp(coupledGradient("porepressure")),
    _fluid_weight(getParam<RealVectorValue>("fluid_weight")),
    _fluid_viscosity(getParam<Real>("fluid_viscosity")),
    _poro_recip(1.0 / getParam<Real>("velocity_scaling")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _component(getParam<MooseEnum>("component"))
{
}

Real
DarcyFluxComponent::computeValue()
{
  return -_poro_recip *
         (_permeability[_qp] * (_grad_pp[_qp] - _fluid_weight) / _fluid_viscosity)(_component);
}
