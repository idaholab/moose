//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowFluidMass.h"

registerMooseObject("PorousFlowApp", FVPorousFlowFluidMass);

InputParameters
FVPorousFlowFluidMass::validParams()
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

FVPorousFlowFluidMass::FVPorousFlowFluidMass(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _fluid_component(getParam<unsigned int>("fluid_component")),
    _phase_index(getParam<std::vector<unsigned int>>("phase")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _has_total_strain(hasMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain(_has_total_strain
                      ? &getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")
                      : nullptr),
    _porosity(getADMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _fluid_density(getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _fluid_saturation(getADMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),
    _mass_fraction(
        getADMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _saturation_threshold(getParam<Real>("saturation_threshold"))
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

  // Error if a strain base_name is provided but doesn't exist
  if (parameters.isParamSetByUser("base_name") && !_has_total_strain)
    paramError("base_name", "A strain base_name ", _base_name, " does not exist");
}

Real
FVPorousFlowFluidMass::computeQpIntegral()
{
  // The fluid mass for the finite volume case is much simpler
  Real mass = 0.0;
  const Real strain = (_has_total_strain ? (*_total_strain)[_qp].trace() : 0.0);

  for (auto ph : _phase_index)
  {
    if (MetaPhysicL::raw_value(_fluid_saturation[_qp][ph]) <= _saturation_threshold)
      mass += MetaPhysicL::raw_value(_fluid_density[_qp][ph] * _fluid_saturation[_qp][ph] *
                                     _mass_fraction[_qp][ph][_fluid_component]);
  }

  return MetaPhysicL::raw_value(_porosity[_qp]) * (1.0 + strain) * mass;
}
