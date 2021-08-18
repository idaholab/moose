//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFluidMass.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowFluidMass);

InputParameters
PorousFlowFluidMass::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addParam<unsigned int>(
      "fluid_component",
      0,
      "The index corresponding to the fluid component that this Postprocessor acts on");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<std::vector<unsigned int>>("phase",
                                             "The index of the fluid phase that this "
                                             "Postprocessor is restricted to.  Multiple "
                                             "indices can be entered");
  params.addRangeCheckedParam<Real>("saturation_threshold",
                                    1.0,
                                    "saturation_threshold >= 0 & saturation_threshold <= 1",
                                    "The saturation threshold below which the mass is calculated "
                                    "for a specific phase. Default is 1.0. Note: only one "
                                    "phase_index can be entered");
  params.addParam<unsigned int>("kernel_variable_number",
                                0,
                                "The PorousFlow variable number (according to the dictator) of "
                                "the fluid-mass kernel.  This is required only in the unusual "
                                "situation where a variety of different finite-element "
                                "interpolation schemes are employed in the simulation");
  params.addParam<std::string>(
      "base_name",
      "For non-mechanically-coupled systems with no TensorMechanics strain calculators, base_name "
      "need not be set.  For mechanically-coupled systems, base_name should be the same base_name "
      "as given to the TensorMechanics object that computes strain, so that this Postprocessor can "
      "correctly account for changes in mesh volume.  For non-mechanically-coupled systems, "
      "base_name should not be the base_name of any TensorMechanics strain calculators.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addClassDescription("Calculates the mass of a fluid component in a region");
  return params;
}

PorousFlowFluidMass::PorousFlowFluidMass(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),

    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _phase_index(getParam<std::vector<unsigned int>>("phase")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _has_total_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain(_has_total_strain
                      ? &getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")
                      : nullptr),
    _porosity(getMaterialProperty<Real>("PorousFlow_porosity_nodal")),
    _fluid_density(getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")),
    _fluid_saturation(getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")),
    _mass_fraction(
        getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")),
    _saturation_threshold(getParam<Real>("saturation_threshold")),
    _var(getParam<unsigned>("kernel_variable_number") < _dictator.numVariables()
             ? &_fe_problem.getStandardVariable(
                   _tid,
                   _dictator
                       .getCoupledStandardMooseVars()[getParam<unsigned>("kernel_variable_number")]
                       ->name())
             : nullptr)
{
  const unsigned int num_phases = _dictator.numPhases();
  const unsigned int num_components = _dictator.numComponents();

  // Check that the number of components entered is not greater than the total number of components
  if (_fluid_component >= num_components)
    paramError(
        "fluid_component",
        "The Dictator proclaims that the number of components in this simulation is ",
        num_components,
        " whereas you have used a component index of ",
        _fluid_component,
        ". Remember that indexing starts at 0. The Dictator does not take such mistakes lightly.");

  // Check that the number of phases entered is not more than the total possible phases
  if (_phase_index.size() > num_phases)
    paramError("phase",
               "The Dictator decrees that the number of phases in this simulation is ",
               num_phases,
               " but you have entered ",
               _phase_index.size(),
               " phases.");

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

  // Also check that the phase indices entered are not greater than the number of phases
  // to avoid a segfault. Note that the input parser takes care of negative inputs so we
  // don't need to guard against them
  if (!_phase_index.empty())
  {
    unsigned int max_phase_num = *std::max_element(_phase_index.begin(), _phase_index.end());
    if (max_phase_num > num_phases - 1)
      paramError("phase",
                 "The Dictator proclaims that the phase index ",
                 max_phase_num,
                 " is greater than the largest phase index possible, which is ",
                 num_phases - 1);
  }

  // Using saturation_threshold only makes sense for a specific phase_index
  if (_saturation_threshold < 1.0 && _phase_index.size() != 1)
    paramError("saturation_threshold",
               "A single phase_index must be entered when prescribing a saturation_threshold");

  // If _phase_index is empty, create vector of all phase numbers to calculate mass over all phases
  if (_phase_index.empty())
    for (unsigned int i = 0; i < num_phases; ++i)
      _phase_index.push_back(i);
}

Real
PorousFlowFluidMass::computeIntegral()
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

    Real mass = 0.0;
    for (auto ph : _phase_index)
    {
      if (_fluid_saturation[node][ph] <= _saturation_threshold)
        mass += _fluid_density[node][ph] * _fluid_saturation[node][ph] *
                _mass_fraction[node][ph][_fluid_component];
    }
    sum += nodal_volume * _porosity[node] * mass;
  }

  return sum;
}

Real
PorousFlowFluidMass::computeQpIntegral()
{
  return 0.0;
}
