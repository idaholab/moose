/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowHeatEnergy.h"

// libmesh includes for qrule
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<PorousFlowHeatEnergy>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<bool>("include_porous_skeleton", true, "Include the heat energy of the porous skeleton");
  params.addParam<std::vector<unsigned int> >("phase", "The index(es) of the fluid phase that this Postprocessor is restricted to.  Multiple indices can be entered.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<unsigned int>("kernel_variable_number", 0, "The PorousFlow variable number (according to the dictatory) of the heat-energy kernel.  This is required only in the unusual situation where a variety of different finite-element interpolation schemes are employed in the simulation");
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
    _fluid_density(_fluid_present ? &getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_density_nodal") : nullptr),
    _fluid_saturation_nodal(_fluid_present ? &getMaterialProperty<std::vector<Real> >("PorousFlow_saturation_nodal") : nullptr),
    _energy_nodal(_fluid_present ? &getMaterialProperty<std::vector<Real> >("PorousFlow_fluid_phase_internal_energy_nodal") : nullptr),
    _var(getParam<unsigned>("kernel_variable_number") < _dictator.numVariables() ? _dictator.getCoupledMooseVars()[getParam<unsigned>("kernel_variable_number")] : nullptr)
{
  if (!_phase_index.empty())
  {
    /// Check that the phase indices entered are not greater than the number of phases
    const unsigned int max_phase_num = *std::max_element(_phase_index.begin(), _phase_index.end());
    if (max_phase_num > _num_phases - 1)
      mooseError("The Dictator proclaims that the phase index " << max_phase_num << " in the Postprocessor " << _name << " is greater than the largest phase index possible, which is " << _num_phases - 1);
  }

  /// Check that kernel_variable_number is OK
  if (getParam<unsigned>("kernel_variable_number") >= _dictator.numVariables())
    mooseError("PorousFlowHeatEnergy: The dictator pronounces that the number of porous-flow variables is " << _dictator.numVariables() << ", however you have used kernel_variable_number = " << getParam<unsigned>("kernel_variable_number") << ".  This is an error");
}

Real
PorousFlowHeatEnergy::computeIntegral()
{
  Real sum = 0;

  /** The use of _test in the loops below mean that the
   * integral is exactly the same as the one computed
   * by the PorousFlowMassTimeDerivative Kernel.  Because that
   * Kernel is lumped, this Postprocessor also needs to
   * be lumped.  Hence the use of the "nodal" Material
   * Properties
   */
  const VariableTestValue & _test = (*_var).phi();

  for (unsigned node = 0 ; node < _test.size(); ++node)
  {
    Real nodal_volume = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      nodal_volume += _JxW[_qp] * _coord[_qp] * _test[node][_qp];

    Real energy = 0.0;
    if (_include_porous_skeleton)
      energy += (1.0 - _porosity[node]) * _rock_energy_nodal[node];

    for (auto ph : _phase_index)
      energy += (*_fluid_density)[node][ph] * (*_fluid_saturation_nodal)[node][ph] * (*_energy_nodal)[node][ph] * _porosity[node];

    sum += nodal_volume * energy;
  }

  return sum;
}

Real
PorousFlowHeatEnergy::computeQpIntegral()
{
  return 0.0;
}
