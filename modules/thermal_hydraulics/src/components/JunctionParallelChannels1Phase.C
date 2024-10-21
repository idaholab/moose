//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JunctionParallelChannels1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", JunctionParallelChannels1Phase);

InputParameters
JunctionParallelChannels1Phase::validParams()
{
  InputParameters params = VolumeJunction1Phase::validParams();

  params.addClassDescription("Junction between 1-phase flow channels that are parallel");

  return params;
}

JunctionParallelChannels1Phase::JunctionParallelChannels1Phase(const InputParameters & params)
  : VolumeJunction1Phase(params)
{
}

void
JunctionParallelChannels1Phase::buildVolumeJunctionUserObject()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADJunctionParallelChannels1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<bool>("use_scalar_variables") = _use_scalar_variables;
    if (!_use_scalar_variables)
      params.set<subdomain_id_type>("junction_subdomain_id") = _junction_subdomain_id;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = getConnectedProcessorIDs();
    params.set<std::vector<UserObjectName>>("numerical_flux_names") = _numerical_flux_names;
    params.set<Real>("volume") = _volume;
    params.set<std::string>("component_name") = name();
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
    params.set<RealVectorValue>("dir_c0") = _directions[0];
    params.set<Real>("K") = _K;
    params.set<Real>("A_ref") = _A_ref;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("apply_velocity_scaling") = getParam<bool>("apply_velocity_scaling");
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _junction_uo_name, params);
    connectObject(params, _junction_uo_name, "K");
  }
}
