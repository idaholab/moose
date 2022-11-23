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
#include "SinglePhaseFluidProperties.h"
#include "Function.h"

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
JunctionParallelChannels1Phase::addVariables()
{
  auto connected_subdomains = getConnectedSubdomainNames();

  getTHMProblem().addSimVariable(
      true, _rhoV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhoV);
  getTHMProblem().addSimVariable(
      true, _rhouV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhouV);
  getTHMProblem().addSimVariable(
      true, _rhovV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhovV);
  getTHMProblem().addSimVariable(
      true, _rhowV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhowV);
  getTHMProblem().addSimVariable(
      true, _rhoEV_var_name, FEType(FIRST, SCALAR), connected_subdomains, _scaling_factor_rhoEV);

  if (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel_x") &&
      isParamValid("initial_vel_y") && isParamValid("initial_vel_z"))
  {
    Function & initial_p_fn = getTHMProblem().getFunction(getParam<FunctionName>("initial_p"));
    Function & initial_T_fn = getTHMProblem().getFunction(getParam<FunctionName>("initial_T"));
    Function & initial_vel_x_fn =
        getTHMProblem().getFunction(getParam<FunctionName>("initial_vel_x"));
    Function & initial_vel_y_fn =
        getTHMProblem().getFunction(getParam<FunctionName>("initial_vel_y"));
    Function & initial_vel_z_fn =
        getTHMProblem().getFunction(getParam<FunctionName>("initial_vel_z"));

    initial_p_fn.initialSetup();
    initial_T_fn.initialSetup();
    initial_vel_x_fn.initialSetup();
    initial_vel_y_fn.initialSetup();
    initial_vel_z_fn.initialSetup();

    const Real initial_p = initial_p_fn.value(0, _position);
    const Real initial_T = initial_T_fn.value(0, _position);
    const Real initial_vel_x = initial_vel_x_fn.value(0, _position);
    const Real initial_vel_y = initial_vel_y_fn.value(0, _position);
    const Real initial_vel_z = initial_vel_z_fn.value(0, _position);

    SinglePhaseFluidProperties & fp =
        getTHMProblem().getUserObject<SinglePhaseFluidProperties>(_fp_name);
    fp.initialSetup();
    const Real initial_rho = fp.rho_from_p_T(initial_p, initial_T);
    const RealVectorValue vel(initial_vel_x, initial_vel_y, initial_vel_z);
    const Real initial_E = fp.e_from_p_rho(initial_p, initial_rho) + 0.5 * vel * vel;

    getTHMProblem().addConstantScalarIC(_rhoV_var_name, initial_rho * _volume);
    getTHMProblem().addConstantScalarIC(_rhouV_var_name, initial_rho * initial_vel_x * _volume);
    getTHMProblem().addConstantScalarIC(_rhovV_var_name, initial_rho * initial_vel_y * _volume);
    getTHMProblem().addConstantScalarIC(_rhowV_var_name, initial_rho * initial_vel_z * _volume);
    getTHMProblem().addConstantScalarIC(_rhoEV_var_name, initial_rho * initial_E * _volume);
  }
}

void
JunctionParallelChannels1Phase::buildVolumeJunctionUserObject()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADJunctionParallelChannels1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = _proc_ids;
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
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _junction_uo_name, params);
  }
}

void
JunctionParallelChannels1Phase::addMooseObjects()
{
  buildVolumeJunctionUserObject();

  // Add BC to each of the connected flow channels
  for (std::size_t i = 0; i < _boundary_names.size(); i++)
  {
    const std::vector<NonlinearVariableName> var_names = {
        FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};
    for (std::size_t j = 0; j < var_names.size(); j++)
    {
      const std::string class_name = "ADVolumeJunction1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_boundary_names[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var_names[j];
      params.set<UserObjectName>("volume_junction_uo") = _junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
      params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
      params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
      params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
      params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
      params.set<bool>("implicit") = getTHMProblem().getImplicitTimeIntegrationFlag();
      getTHMProblem().addBoundaryCondition(
          class_name, genName(name(), i, var_names[j] + ":" + class_name), params);
    }
  }

  // Add scalar kernels for the junction
  std::vector<NonlinearVariableName> var_names(N_EQ);
  var_names[RHOV_INDEX] = _rhoV_var_name;
  var_names[RHOUV_INDEX] = _rhouV_var_name;
  var_names[RHOVV_INDEX] = _rhovV_var_name;
  var_names[RHOWV_INDEX] = _rhowV_var_name;
  var_names[RHOEV_INDEX] = _rhoEV_var_name;
  for (std::size_t i = 0; i < N_EQ; i++)
  {
    {
      const std::string class_name = "ODETimeDerivative";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      getTHMProblem().addScalarKernel(class_name, genName(name(), var_names[i], "td"), params);
    }
    {
      const std::string class_name = "ADVolumeJunctionAdvectionScalarKernel";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = var_names[i];
      params.set<UserObjectName>("volume_junction_uo") = _junction_uo_name;
      params.set<unsigned int>("equation_index") = i;
      getTHMProblem().addScalarKernel(class_name, genName(name(), var_names[i], "vja_sk"), params);
    }
  }
}
