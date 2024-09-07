//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowHeatEnergy.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowHeatEnergy);

InputParameters
PorousFlowHeatEnergy::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<bool>(
      "include_porous_skeleton", true, "Include the heat energy of the porous skeleton");
  params.addParam<std::vector<unsigned int>>("phase",
                                             {},
                                             "The index(es) of the fluid phase that this "
                                             "Postprocessor is restricted to.  Multiple "
                                             "indices can be entered.");
  params.addParam<std::string>(
      "base_name",
      "For non-mechanically-coupled systems with no TensorMechanics strain calculators, base_name "
      "need not be set.  For mechanically-coupled systems, base_name should be the same base_name "
      "as given to the TensorMechanics object that computes strain, so that this Postprocessor can "
      "correctly account for changes in mesh volume.  For non-mechanically-coupled systems, "
      "base_name should not be the base_name of any TensorMechanics strain calculators.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addParam<unsigned int>("kernel_variable_number",
                                0,
                                "The PorousFlow variable number (according to the dictatory) of "
                                "the heat-energy kernel.  This is required only in the unusual "
                                "situation where a variety of different finite-element "
                                "interpolation schemes are employed in the simulation");
  params.addClassDescription("Calculates the sum of heat energy of fluid phase(s) and/or the "
                             "porous skeleton in a region");
  return params;
}

PorousFlowHeatEnergy::PorousFlowHeatEnergy(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _has_total_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain(_has_total_strain
                      ? &getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")
                      : nullptr),
    _fluid_present(_num_phases > 0),
    _include_porous_skeleton(getParam<bool>("include_porous_skeleton")),
    _phase_index(getParam<std::vector<unsigned int>>("phase")),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _rock_energy_nodal(getMaterialProperty<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _fluid_density(_fluid_present ? &getMaterialProperty<std::vector<Real>>(
                                        "PorousFlow_fluid_phase_density_nodal")
                                  : nullptr),
    _fluid_saturation_nodal(
        _fluid_present ? &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                       : nullptr),
    _energy_nodal(_fluid_present ? &getMaterialProperty<std::vector<Real>>(
                                       "PorousFlow_fluid_phase_internal_energy_nodal")
                                 : nullptr),
    _var(getParam<unsigned>("kernel_variable_number") < _dictator.numVariables()
             ? &_fe_problem.getStandardVariable(
                   _tid,
                   _dictator
                       .getCoupledStandardMooseVars()[getParam<unsigned>("kernel_variable_number")]
                       ->name())
             : nullptr)
{
  if (!_phase_index.empty())
  {
    // Check that the phase indices entered are not greater than the number of phases
    const unsigned int max_phase_num = *std::max_element(_phase_index.begin(), _phase_index.end());
    if (max_phase_num > _num_phases - 1)
      paramError("phase",
                 "The Dictator proclaims that the phase index ",
                 max_phase_num,
                 " is greater than the largest phase index possible, which is ",
                 _num_phases - 1);
  }

  // Check that kernel_variable_number is OK
  if (getParam<unsigned>("kernel_variable_number") >= _dictator.numVariables())
    paramError("kernel_variable_number",
               "The Dictator pronounces that the number of PorousFlow variables is ",
               _dictator.numVariables(),
               ", however you have used ",
               getParam<unsigned>("kernel_variable_number"),
               ". This is an error");

  // Now that we know kernel_variable_number is OK, _var must be OK,
  // so ensure that reinit is called on _var:
  addMooseVariableDependency(_var);

  // Error if a strain base_name is provided but doesn't exist
  if (parameters.isParamSetByUser("base_name") && !_has_total_strain)
    paramError("base_name", "A strain base_name ", _base_name, " does not exist");
}

Real
PorousFlowHeatEnergy::computeIntegral()
{
  Real sum = 0;

  // The use of _test in the loops below mean that the
  // integral is exactly the same as the one computed
  // by the PorousFlowMassTimeDerivative Kernel.  Because that
  // Kernel is lumped, this Postprocessor also needs to
  // be lumped.  Hence the use of the "nodal" Material
  // Properties
  const VariableTestValue & test = _var->phi();

  for (unsigned node = 0; node < test.size(); ++node)
  {
    Real nodal_volume = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      const Real n_v = _JxW[_qp] * _coord[_qp] * test[node][_qp];
      if (_has_total_strain)
        nodal_volume += n_v * (1.0 + (*_total_strain)[_qp].trace());
      else
        nodal_volume += n_v;
    }

    Real energy = 0.0;
    if (_include_porous_skeleton)
      energy += (1.0 - _porosity[node]) * _rock_energy_nodal[node];

    for (auto ph : _phase_index)
      energy += (*_fluid_density)[node][ph] * (*_fluid_saturation_nodal)[node][ph] *
                (*_energy_nodal)[node][ph] * _porosity[node];

    sum += nodal_volume * energy;
  }

  return sum;
}

Real
PorousFlowHeatEnergy::computeQpIntegral()
{
  return 0.0;
}
