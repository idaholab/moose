/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


//  This post processor returns the component mass in a region.
#include "PorousFlowFluidMass.h"

template<>
InputParameters validParams<PorousFlowFluidMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addParam<unsigned int>("component_index", 0, "The index corresponding to the component for this kernel");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("Returns the mass in a region.");
  return params;
}

PorousFlowFluidMass::PorousFlowFluidMass(const InputParameters & parameters) :
  ElementIntegralVariablePostprocessor(parameters),

  _component_index(getParam<unsigned int>("component_index")),
  _dictator_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),

  _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
  _fluid_density(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density")),
  _fluid_saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
  _mass_frac(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac"))
{
  if (_component_index >= _dictator_UO.num_components())
    mooseError("The Dictator proclaims that the number of components in this simulation is " << _dictator_UO.num_components() << " whereas you have used the Postprocessor PorousFlowFluidMass with component = " << _component_index << ".  The Dictator does not take such mistakes lightly");
}

Real
PorousFlowFluidMass::computeQpIntegral()
{
  Real mass = 0.0;
  for (unsigned ph = 0; ph < _dictator_UO.num_phases(); ++ph)
    mass += _fluid_density[_qp][ph] * _fluid_saturation[_qp][ph] * _mass_frac[_qp][ph][_component_index];

  return _porosity[_qp] * mass;
}
