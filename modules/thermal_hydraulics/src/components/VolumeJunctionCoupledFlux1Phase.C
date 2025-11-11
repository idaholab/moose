//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunctionCoupledFlux1Phase.h"
#include "VolumeJunction1Phase.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunctionCoupledFlux1Phase);

InputParameters
VolumeJunctionCoupledFlux1Phase::validParams()
{
  InputParameters params = Component::validParams();

  params.addRequiredParam<std::string>("volume_junction",
                                       "Name of volume junction component on which to apply flux");
  params.addRequiredParam<RealVectorValue>("normal_from_junction",
                                           "Direction vector pointing from the junction to the "
                                           "coupled volume. This vector will be normalized.");
  params.addRequiredParam<Real>("A_coupled", "Area of the coupled surface [m^2]");
  params.addRequiredParam<MultiAppName>("multi_app", "MultiApp to transfer to and from");
  params.addRequiredParam<std::string>("pp_suffix", "Suffix to append to post-processor names");

  params.addClassDescription(
      "Applies a flux between a VolumeJunction1Phase component and an external application.");

  return params;
}

VolumeJunctionCoupledFlux1Phase::VolumeJunctionCoupledFlux1Phase(const InputParameters & params)
  : Component(params),
    _volume_junction_name(getParam<std::string>("volume_junction")),
    _normal_from_junction_unnormalized(getParam<RealVectorValue>("normal_from_junction")),
    _normal_from_junction(_normal_from_junction_unnormalized.unit()),
    _multi_app_name(getParam<MultiAppName>("multi_app")),
    _pp_suffix(getParam<std::string>("pp_suffix"))
{
}

void
VolumeJunctionCoupledFlux1Phase::check() const
{
  checkComponentOfTypeExistsByName<VolumeJunction1Phase>(_volume_junction_name);
}

void
VolumeJunctionCoupledFlux1Phase::addMooseObjects()
{
  const std::vector<NonlinearVariableName> vars = {"rhoV", "rhouV", "rhovV", "rhowV", "rhoEV"};
  for (const auto i : index_range(vars))
    addVolumeJunctionKernel(vars[i], i);

  const std::vector<std::string> equations = {"mass", "energy"};
  for (const auto i : index_range(equations))
  {
    addFluxPostprocessor(equations[i]);
    addFluxTransfer(equations[i]);
  }

  const std::vector<std::string> properties = {"p", "T"};
  for (const auto i : index_range(properties))
  {
    addPropertyPostprocessor(properties[i]);
    addPropertyTransfer(properties[i]);
  }
}

void
VolumeJunctionCoupledFlux1Phase::addVolumeJunctionKernel(const std::string & var, unsigned int i)
{
  const auto & volume_junction =
      getTHMProblem().getComponentByName<VolumeJunction1Phase>(_volume_junction_name);

  const std::string class_name = "VolumeJunctionCoupledFlux1PhaseKernel";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = var;
  params.set<std::vector<SubdomainName>>("block") = {_volume_junction_name};
  params.set<unsigned int>("equation_index") = i;
  params.set<PostprocessorName>("pressure") = addPostprocessorSuffix("p");
  params.set<PostprocessorName>("temperature") = addPostprocessorSuffix("T");
  params.set<Real>("A_coupled") = getParam<Real>("A_coupled");
  params.set<RealVectorValue>("normal_from_junction") = _normal_from_junction;
  params.set<UserObjectName>("volume_junction_uo") =
      volume_junction.getVolumeJunctionUserObjectName();
  params.set<UserObjectName>("numerical_flux_uo") = volume_junction.getNumericalFluxName(0);
  params.set<UserObjectName>("fluid_properties") = volume_junction.getFluidPropertiesName();
  const std::string obj_name = genName(name(), var + "_kernel");
  getTHMProblem().addKernel(class_name, obj_name, params);
}

void
VolumeJunctionCoupledFlux1Phase::addFluxPostprocessor(const std::string & equation)
{
  const auto & volume_junction =
      getTHMProblem().getComponentByName<VolumeJunction1Phase>(_volume_junction_name);

  const std::string quantity = equation + "_rate";
  const std::string class_name = "VolumeJunctionCoupledFlux1PhasePostprocessor";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<MooseEnum>("equation") = equation;
  params.set<PostprocessorName>("pressure") = addPostprocessorSuffix("p");
  params.set<PostprocessorName>("temperature") = addPostprocessorSuffix("T");
  params.set<Real>("A_coupled") = getParam<Real>("A_coupled");
  params.set<RealVectorValue>("normal_from_junction") = _normal_from_junction;
  params.set<UserObjectName>("volume_junction_uo") =
      volume_junction.getVolumeJunctionUserObjectName();
  params.set<UserObjectName>("numerical_flux_uo") = volume_junction.getNumericalFluxName(0);
  params.set<UserObjectName>("fluid_properties") = volume_junction.getFluidPropertiesName();
  const std::string obj_name = addPostprocessorSuffix(quantity);
  getTHMProblem().addPostprocessor(class_name, obj_name, params);
}

void
VolumeJunctionCoupledFlux1Phase::addPropertyPostprocessor(const std::string & property)
{
  const std::string class_name = "Receiver";
  InputParameters params = _factory.getValidParams(class_name);
  const PostprocessorName obj_name = addPostprocessorSuffix(property);
  getTHMProblem().addPostprocessor(class_name, obj_name, params);
}

void
VolumeJunctionCoupledFlux1Phase::addFluxTransfer(const std::string & equation)
{
  const std::string quantity = equation + "_rate";
  const std::string class_name = "MultiAppPostprocessorTransfer";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<MultiAppName>("to_multi_app") = _multi_app_name;
  params.set<PostprocessorName>("from_postprocessor") = addPostprocessorSuffix(quantity);
  params.set<PostprocessorName>("to_postprocessor") = addPostprocessorSuffix(quantity);
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  const PostprocessorName obj_name = genName(name(), quantity + "_transfer");
  getTHMProblem().addTransfer(class_name, obj_name, params);
}

void
VolumeJunctionCoupledFlux1Phase::addPropertyTransfer(const std::string & property)
{
  const std::string class_name = "MultiAppPostprocessorTransfer";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<MultiAppName>("from_multi_app") = _multi_app_name;
  params.set<PostprocessorName>("from_postprocessor") = addPostprocessorSuffix(property);
  params.set<PostprocessorName>("to_postprocessor") = addPostprocessorSuffix(property);
  params.set<MooseEnum>("reduction_type") = "average";
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  const PostprocessorName obj_name = genName(name(), property + "_transfer");
  getTHMProblem().addTransfer(class_name, obj_name, params);
}

PostprocessorName
VolumeJunctionCoupledFlux1Phase::addPostprocessorSuffix(const std::string & base_name) const
{
  return base_name + "_" + _pp_suffix;
}
