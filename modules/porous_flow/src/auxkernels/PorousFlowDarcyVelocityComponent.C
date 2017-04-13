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
#include "PorousFlowDarcyVelocityComponent.h"

template <>
InputParameters
validParams<PorousFlowDarcyVelocityComponent>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>("fluid_phase", 0, "The index corresponding to the fluid phase");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The spatial component of the Darcy flux to return");
  params.addClassDescription("Darcy velocity (in m^3.s^-1.m^-2, or m.s^-1)  -(k_ij * krel /mu "
                             "(nabla_j P - w_j)), where k_ij is the permeability tensor, krel is "
                             "the relative permeability, mu is the fluid viscosity, P is the fluid "
                             "pressure, and w_j is the fluid weight.");
  return params;
}

PorousFlowDarcyVelocityComponent::PorousFlowDarcyVelocityComponent(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _relative_permeability(
        getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_qp")),
    _fluid_viscosity(getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _permeability(getMaterialProperty<RealTensorValue>("PorousFlow_permeability_qp")),
    _grad_p(getMaterialProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _fluid_density_qp(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _ph(getParam<unsigned int>("fluid_phase")),
    _component(getParam<MooseEnum>("component")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
  if (_ph >= _dictator.numPhases())
    mooseError(
        "The Dictator proclaims that the number of phases in this simulation is ",
        _dictator.numPhases(),
        " whereas you have used the AuxKernel PorousFlowDarcyVelocityComponent with fluid_phase = ",
        _ph,
        ".  The Dictator is watching you, to ensure your wellbeing.");
}

Real
PorousFlowDarcyVelocityComponent::computeValue()
{
  // note that in the following, _relative_permeaility and _fluid_viscosity are upwinded (nodal)
  // values
  return -(_permeability[_qp] * (_grad_p[_qp][_ph] - _fluid_density_qp[_qp][_ph] * _gravity) *
           _relative_permeability[_qp][_ph] / _fluid_viscosity[_qp][_ph])(_component);
}
