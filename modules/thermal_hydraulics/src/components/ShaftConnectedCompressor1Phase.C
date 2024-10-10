//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedCompressor1Phase.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"
#include "Shaft.h"
#include "ADShaftConnectedCompressor1PhaseUserObject.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"
#include "ScalarKernel.h"
#include "ADVolumeJunction1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedCompressor1Phase);

InputParameters
ShaftConnectedCompressor1Phase::validParams()
{
  InputParameters params = VolumeJunction1Phase::validParams();
  params += ShaftConnectable::validParams();
  params.makeParamRequired<Real>("A_ref");
  params.addRequiredParam<BoundaryName>("inlet", "Compressor inlet");
  params.addRequiredParam<BoundaryName>("outlet", "Compressor outlet");
  params.set<std::vector<BoundaryName>>("connections") = {};
  params.suppressParameter<std::vector<BoundaryName>>("connections");
  params.addParam<bool>("treat_as_turbine", false, "Treat the compressor as a turbine?");
  params.addRequiredParam<Real>("omega_rated", "Rated compressor speed [rad/s]");
  params.addRequiredParam<Real>("mdot_rated", "Rated compressor mass flow rate [kg/s]");
  params.addRequiredParam<Real>("rho0_rated", "Rated compressor stagnation fluid density [kg/m^3]");
  params.addRequiredParam<Real>("c0_rated", "Rated compressor stagnation sound speed [m/s]");
  params.addRequiredParam<Real>("speed_cr_fr", "Compressor speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Compressor friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff",
                                             "Compressor friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Compressor speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Compressor inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff",
                                             "Compressor inertia coefficients [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>(
      "speeds",
      "Relative corrected speeds. Order of speeds needs correspond to the "
      "orders of `Rp_functions` and `eff_functions` [-]");
  params.addRequiredParam<std::vector<FunctionName>>(
      "Rp_functions",
      "Functions of pressure ratio versus relative corrected flow. Each function is for a "
      "different, constant relative corrected speed. The order of function names should correspond "
      "to the order of speeds in the `speeds` parameter [-]");
  params.addRequiredParam<std::vector<FunctionName>>(
      "eff_functions",
      "Functions of adiabatic efficiency versus relative corrected flow. Each function is for a "
      "different, constant relative corrected speed. The order of function names should correspond "
      "to the order of speeds in the `speeds` parameter [-]");
  params.addParam<Real>("min_pressure_ratio", 0.0, "Minimum pressure ratio");
  params.addParam<Real>("max_pressure_ratio", 50.0, "Maximum pressure ratio");

  params.addClassDescription(
      "1-phase compressor that must be connected to a Shaft component. Compressor speed "
      "is controlled by the connected shaft; Isentropic/Dissipation torque and delta_p are "
      "computed by user input functions of inlet flow rate and shaft speed");

  return params;
}

ShaftConnectedCompressor1Phase::ShaftConnectedCompressor1Phase(const InputParameters & parameters)
  : VolumeJunction1Phase(parameters),
    ShaftConnectable(this),
    _inlet(getParam<BoundaryName>("inlet")),
    _outlet(getParam<BoundaryName>("outlet")),
    _omega_rated(getParam<Real>("omega_rated")),
    _mdot_rated(getParam<Real>("mdot_rated")),
    _rho0_rated(getParam<Real>("rho0_rated")),
    _c0_rated(getParam<Real>("c0_rated")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _speeds(getParam<std::vector<Real>>("speeds")),
    _Rp_functions(getParam<std::vector<FunctionName>>("Rp_functions")),
    _eff_functions(getParam<std::vector<FunctionName>>("eff_functions")),
    _delta_p_var_name(junctionVariableName("delta_p")),
    _isentropic_torque_var_name(junctionVariableName("isentropic_torque")),
    _dissipation_torque_var_name(junctionVariableName("dissipation_torque")),
    _friction_torque_var_name(junctionVariableName("friction_torque")),
    _moi_var_name(junctionVariableName("moment_of_inertia"))
{
  // this determines connection ordering
  addConnection(_inlet);
  addConnection(_outlet);

  checkSizeEqualsValue<Real>("tau_fr_coeff", 4);
  checkSizeEqualsValue<Real>("inertia_coeff", 4);
}

void
ShaftConnectedCompressor1Phase::check() const
{
  VolumeJunction1Phase::check();
  checkShaftConnection(this);
}

void
ShaftConnectedCompressor1Phase::buildVolumeJunctionUserObject()
{
  const Component & c = getComponentByName<Component>(_shaft_name);
  const Shaft & scc = dynamic_cast<const Shaft &>(c);
  const VariableName omega_var_name = scc.getOmegaVariableName();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADShaftConnectedCompressor1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<bool>("use_scalar_variables") = _use_scalar_variables;
    if (!_use_scalar_variables)
      params.set<subdomain_id_type>("junction_subdomain_id") = _junction_subdomain_id;
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = getConnectedProcessorIDs();
    params.set<std::vector<UserObjectName>>("numerical_flux_names") = _numerical_flux_names;
    params.set<Real>("volume") = _volume;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("rhoV") = {_rhoV_var_name};
    params.set<std::vector<VariableName>>("rhouV") = {_rhouV_var_name};
    params.set<std::vector<VariableName>>("rhovV") = {_rhovV_var_name};
    params.set<std::vector<VariableName>>("rhowV") = {_rhowV_var_name};
    params.set<std::vector<VariableName>>("rhoEV") = {_rhoEV_var_name};
    // the direction of the outlet channel
    params.set<Point>("di_out") = _directions[1].unit();
    params.set<Real>("omega_rated") = _omega_rated;
    params.set<bool>("treat_as_turbine") = getParam<bool>("treat_as_turbine");
    params.set<Real>("mdot_rated") = _mdot_rated;
    params.set<Real>("rho0_rated") = _rho0_rated;
    params.set<Real>("c0_rated") = _c0_rated;
    params.set<Real>("speed_cr_fr") = _speed_cr_fr;
    params.set<Real>("tau_fr_const") = _tau_fr_const;
    params.set<std::vector<Real>>("tau_fr_coeff") = _tau_fr_coeff;
    params.set<Real>("speed_cr_I") = _speed_cr_I;
    params.set<Real>("inertia_const") = _inertia_const;
    params.set<std::vector<Real>>("inertia_coeff") = _inertia_coeff;
    params.set<std::vector<Real>>("speeds") = _speeds;
    params.set<std::vector<FunctionName>>("Rp_functions") = _Rp_functions;
    params.set<std::vector<FunctionName>>("eff_functions") = _eff_functions;
    params.set<Real>("min_pressure_ratio") = getParam<Real>("min_pressure_ratio");
    params.set<Real>("max_pressure_ratio") = getParam<Real>("max_pressure_ratio");
    params.set<std::vector<VariableName>>("omega") = {omega_var_name};
    params.set<Real>("A_ref") = getParam<Real>("A_ref");
    params.set<Real>("K") = getParam<Real>("K");
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::string>("compressor_name") = cname();
    params.set<bool>("apply_velocity_scaling") = getParam<bool>("apply_velocity_scaling");
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, getShaftConnectedUserObjectName(), params);
    connectObject(params, _junction_uo_name, "K");
  }
}

void
ShaftConnectedCompressor1Phase::addVariables()
{
  VolumeJunction1Phase::addVariables();

  addJunctionVariable(false, _delta_p_var_name);
  addJunctionVariable(false, _isentropic_torque_var_name);
  addJunctionVariable(false, _dissipation_torque_var_name);
  addJunctionVariable(false, _friction_torque_var_name);
  addJunctionVariable(false, _moment_of_inertia_var_name);

  if (!_app.isRestarting())
  {
    addJunctionIC(_delta_p_var_name, 0);
    addJunctionIC(_isentropic_torque_var_name, 0);
    addJunctionIC(_dissipation_torque_var_name, 0);
    addJunctionIC(_friction_torque_var_name, 0);
    addJunctionIC(_moment_of_inertia_var_name, _inertia_const);
  }
}

void
ShaftConnectedCompressor1Phase::addMooseObjects()
{
  VolumeJunction1Phase::addMooseObjects();

  const std::vector<std::pair<std::string, VariableName>> quantities_aux = {
      {"delta_p", _delta_p_var_name},
      {"isentropic_torque", _isentropic_torque_var_name},
      {"dissipation_torque", _dissipation_torque_var_name},
      {"friction_torque", _friction_torque_var_name},
      {"moment_of_inertia", _moment_of_inertia_var_name}};
  for (const auto & quantity_and_name : quantities_aux)
  {
    const std::string class_name = _use_scalar_variables ? "ShaftConnectedCompressor1PhaseScalarAux"
                                                         : "ShaftConnectedCompressor1PhaseAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = quantity_and_name.second;
    params.set<MooseEnum>("quantity") = quantity_and_name.first;
    params.set<UserObjectName>("compressor_uo") = getShaftConnectedUserObjectName();
    const std::string obj_name = genName(name(), quantity_and_name.first + "_aux");
    if (_use_scalar_variables)
      getTHMProblem().addAuxScalarKernel(class_name, obj_name, params);
    else
    {
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      getTHMProblem().addAuxKernel(class_name, obj_name, params);
    }
  }

  const std::vector<std::string> quantities_pp = {
      "pressure_ratio", "efficiency", "rel_corrected_flow", "rel_corrected_speed"};
  for (const auto & quantity : quantities_pp)
  {
    const std::string class_name = "ShaftConnectedCompressor1PhasePostprocessor";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseEnum>("quantity") = quantity;
    params.set<UserObjectName>("compressor_uo") = getShaftConnectedUserObjectName();
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    getTHMProblem().addPostprocessor(class_name, Component::genName(name(), quantity), params);
  }
}
