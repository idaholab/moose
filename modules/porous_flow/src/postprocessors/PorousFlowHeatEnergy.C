/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowHeatEnergy.h"

template<>
InputParameters validParams<PorousFlowHeatEnergy>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<bool>("include_porous_skeleton", true, "Include the heat energy of the porous skeleton");
  params.addParam<std::vector<unsigned int> >("phase", "The index(es) of the fluid phase that this Postprocessor is restricted to.  Multiple indices can be entered.");
  params.addClassDescription("Calculates the sum of heat energy of fluid component(s) and/or the porous skeleton in a region");
  return params;
}

PorousFlowHeatEnergy::PorousFlowHeatEnergy(const InputParameters & parameters) :
    ElementIntegralPostprocessor(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _fluid_present(_num_phases > 0),
    _include_porous_skeleton(getParam<bool>("include_porous_skeleton")),
    _phase_index(getParam<std::vector<unsigned int> >("phase")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _rock_energy_nodal(getMaterialProperty<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _fluid_density(_fluid_present ? &getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density") : NULL),
    _fluid_saturation_nodal(_fluid_present ? &getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal") : NULL),
    _energy_nodal(_fluid_present ? &getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_internal_energy_nodal") : NULL)
{
  if (!_phase_index.empty())
  {
    /// Check that the phase indices entered are not greater than the number of phases
    const unsigned int max_phase_num = *std::max_element(_phase_index.begin(), _phase_index.end());
    if (max_phase_num > _num_phases - 1)
      mooseError("The Dictator proclaims that the phase index " << max_phase_num << " in the Postprocessor " << _name << " is greater than the largest phase index possible, which is " << _num_phases - 1);
  }
}

Real
PorousFlowHeatEnergy::computeQpIntegral()
{
  Real energy = 0.0;
  if (_include_porous_skeleton)
    energy += (1.0 - _porosity[_qp]) * _rock_energy_nodal[_qp];

  for (unsigned int i = 0; i < _phase_index.size(); ++i)
  {
    const unsigned ph = _phase_index[i];
    energy += (*_fluid_density)[_qp][ph] * (*_fluid_saturation_nodal)[_qp][ph] * (*_energy_nodal)[_qp][ph] * _porosity[_qp];
  }

  return energy;
}
