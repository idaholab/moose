/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidMass.h"

// libmesh includes for qrule
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<PorousFlowFluidMass>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addParam<unsigned int>("fluid_component", 0, "The index corresponding to the fluid component that this Postprocessor acts on");
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<std::vector<unsigned int> >("phase", "The index of the fluid phase that this Postprocessor is restricted to.  Multiple indices can be entered");
  params.addRangeCheckedParam<Real>("saturation_threshold", 1.0, "saturation_threshold >= 0 & saturation_threshold <= 1", "The saturation threshold below which the mass is calculated for a specific phase. Default is 1.0. Note: only one phase_index can be entered");
  params.addClassDescription("Calculates the mass of a fluid component in a region");
  return params;
}

PorousFlowFluidMass::PorousFlowFluidMass(const InputParameters & parameters) :
  ElementIntegralPostprocessor(parameters),

  _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
  _fluid_component(getParam<unsigned int>("fluid_component")),
  _phase_index(getParam<std::vector<unsigned int> >("phase")),
  _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
  _fluid_density(getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density_nodal")),
  _fluid_saturation(getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal")),
  _mass_fraction(getMaterialProperty<std::vector<std::vector<Real> > >("PorousFlow_mass_frac_nodal")),
  _saturation_threshold(getParam<Real>("saturation_threshold"))
{
  const unsigned int num_phases = _dictator.numPhases();
  const unsigned int num_components = _dictator.numComponents();

  /// Check that the number of components entered is not greater than the total number of components
  if (_fluid_component >= num_components)
    mooseError("The Dictator proclaims that the number of components in this simulation is " << num_components << " whereas you have used the Postprocessor PorousFlowFluidMass with component = " << _fluid_component << ".  The Dictator does not take such mistakes lightly.");

  /// Check that the number of phases entered is not more than the total possible phases
  if (_phase_index.size() > num_phases)
   mooseError("The Dictator decrees that the number of phases in this simulation is " << num_phases << " but you have entered " << _phase_index.size() << " phases in the Postprocessor " << _name);

  /**
   * Also check that the phase indices entered are not greater than the number of phases
   * to avoid a segfault. Note that the input parser takes care of negative inputs so we
   * don't need to guard against them
   */
  if (!_phase_index.empty())
  {
    unsigned int max_phase_num = * std::max_element(_phase_index.begin(), _phase_index.end());
    if (max_phase_num > num_phases - 1)
      mooseError("The Dictator proclaims that the phase index " << max_phase_num << " in the Postprocessor " << _name << " is greater than the largest phase index possible, which is " << num_phases - 1);
  }

  /// Using saturation_threshold only makes sense for a specific phase_index
  if (_saturation_threshold < 1.0 && _phase_index.size() != 1)
    mooseError("A single phase_index must be entered when prescribing a saturation_threshold in the Postprocessor " << _name);

  /// If _phase_index is empty, create vector of all phase numbers to calculate mass over all phases
  if (_phase_index.empty())
    for (unsigned int i = 0; i < num_phases; ++i)
      _phase_index.push_back(i);
}

Real
PorousFlowFluidMass::computeQpIntegral()
{
  mooseAssert(_current_elem->n_nodes() == _qrule->n_points(), "PorousFlow Postprocessors are currently only defined for number nodes = number quadpoints.");

  Real mass = 0.0;
  unsigned int ph;

  for (unsigned int i = 0; i < _phase_index.size(); ++i)
  {
    ph = _phase_index[i];
    if (_fluid_saturation[_qp][ph] <= _saturation_threshold)
      mass += _fluid_density[_qp][ph] * _fluid_saturation[_qp][ph] * _mass_fraction[_qp][ph][_fluid_component];
  }

  return _porosity[_qp] * mass;
}
