/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidMass.h"

template<>
InputParameters validParams<PorousFlowFluidMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addParam<unsigned int>("fluid_component", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("Returns the fluid mass of a given component in a region.");
  return params;
}

PorousFlowFluidMass::PorousFlowFluidMass(const InputParameters & parameters) :
  ElementIntegralVariablePostprocessor(parameters),

  _fluid_component(getParam<unsigned int>("fluid_component")),
  _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),

  _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
  _fluid_density(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _fluid_saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
  _mass_frac(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac"))
{
  if (_fluid_component >= _dictator.numComponents())
    mooseError("The Dictator proclaims that the number of components in this simulation is " << _dictator.numComponents() << " whereas you have used the Postprocessor PorousFlowFluidMass with component = " << _fluid_component << ".  The Dictator does not take such mistakes lightly.");
}

Real
PorousFlowFluidMass::computeQpIntegral()
{
  Real mass = 0.0;
  for (unsigned ph = 0; ph < _dictator.numPhases(); ++ph)
    mass += _fluid_density[_qp][ph] * _fluid_saturation[_qp][ph] * _mass_frac[_qp][ph][_fluid_component];

  return _porosity[_qp] * mass;
}
