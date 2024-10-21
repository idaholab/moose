//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedPump1Phase.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"
#include "Shaft.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"
#include "ScalarKernel.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedPump1Phase);

InputParameters
ShaftConnectedPump1Phase::validParams()
{
  InputParameters params = VolumeJunction1Phase::validParams();
  params += ShaftConnectable::validParams();
  params.makeParamRequired<Real>("A_ref");
  params.addRequiredParam<BoundaryName>("inlet", "Pump inlet");
  params.addRequiredParam<BoundaryName>("outlet", "Pump outlet");
  params.set<std::vector<BoundaryName>>("connections") = {};
  params.suppressParameter<std::vector<BoundaryName>>("connections");
  params.addRequiredParam<Real>("omega_rated", "Rated pump speed [rad/s]");
  params.addRequiredParam<Real>("volumetric_rated", "Rated pump volumetric flow rate [m^3/s]");
  params.addRequiredParam<Real>("head_rated", "Rated pump head [m]");
  params.addRequiredParam<Real>("torque_rated", "Rated pump torque [N-m]");
  params.addRequiredParam<Real>("density_rated", "Rated pump fluid density [kg/m^3]");
  params.addRequiredParam<Real>("speed_cr_fr", "Pump speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Pump friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Pump friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Pump speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Pump inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff", "Pump inertia coefficients [kg-m^2]");
  params.addRequiredParam<FunctionName>("head", "Function to compute data for pump head [-]");
  params.addRequiredParam<FunctionName>("torque_hydraulic",
                                        "Function to compute data for pump torque [-]");
  params.addParam<Real>(
      "transition_width",
      1e-3,
      "Transition width for sign of the frictional torque at 0 speed over rated speed ratio.");

  params.addClassDescription("1-phase pump that must be connected to a Shaft component. Pump speed "
                             "is controlled by the connected shaft; Hydraulic torque and head are "
                             "computed by user input functions of inlet flow rate and shaft speed");

  return params;
}

ShaftConnectedPump1Phase::ShaftConnectedPump1Phase(const InputParameters & parameters)
  : VolumeJunction1Phase(parameters),
    ShaftConnectable(this),
    _inlet(getParam<BoundaryName>("inlet")),
    _outlet(getParam<BoundaryName>("outlet")),
    _omega_rated(getParam<Real>("omega_rated")),
    _volumetric_rated(getParam<Real>("volumetric_rated")),
    _head_rated(getParam<Real>("head_rated")),
    _torque_rated(getParam<Real>("torque_rated")),
    _density_rated(getParam<Real>("density_rated")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _head(getParam<FunctionName>("head")),
    _torque_hydraulic(getParam<FunctionName>("torque_hydraulic")),
    _head_var_name(junctionVariableName("head")),
    _hydraulic_torque_var_name(junctionVariableName("hydraulic_torque")),
    _friction_torque_var_name(junctionVariableName("friction_torque")),
    _moi_var_name(junctionVariableName("moment_of_inertia")),
    _transition_width(getParam<Real>("transition_width"))
{
  // this determines connection ordering
  addConnection(_inlet);
  addConnection(_outlet);

  checkSizeEqualsValue<Real>("tau_fr_coeff", 4);
  checkSizeEqualsValue<Real>("inertia_coeff", 4);
}

void
ShaftConnectedPump1Phase::check() const
{
  VolumeJunction1Phase::check();
  checkShaftConnection(this);
}

void
ShaftConnectedPump1Phase::buildVolumeJunctionUserObject()
{
  const Component & c = getComponentByName<Component>(_shaft_name);
  const Shaft & scc = dynamic_cast<const Shaft &>(c);
  const VariableName omega_var_name = scc.getOmegaVariableName();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADShaftConnectedPump1PhaseUserObject";
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
    params.set<Real>("gravity_magnitude") = THM::gravity_const;
    params.set<Real>("omega_rated") = _omega_rated;
    params.set<Real>("volumetric_rated") = _volumetric_rated;
    params.set<Real>("head_rated") = _head_rated;
    params.set<Real>("torque_rated") = _torque_rated;
    params.set<Real>("density_rated") = _density_rated;
    params.set<Real>("speed_cr_fr") = _speed_cr_fr;
    params.set<Real>("tau_fr_const") = _tau_fr_const;
    params.set<std::vector<Real>>("tau_fr_coeff") = _tau_fr_coeff;
    params.set<Real>("speed_cr_I") = _speed_cr_I;
    params.set<Real>("inertia_const") = _inertia_const;
    params.set<Real>("transition_width") = _transition_width;
    params.set<std::vector<Real>>("inertia_coeff") = _inertia_coeff;
    params.set<FunctionName>("head") = _head;
    params.set<FunctionName>("torque_hydraulic") = _torque_hydraulic;
    params.set<std::vector<VariableName>>("omega") = {omega_var_name};
    params.set<Real>("A_ref") = getParam<Real>("A_ref");
    params.set<Real>("K") = getParam<Real>("K");
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::string>("pump_name") = cname();
    params.set<bool>("apply_velocity_scaling") = getParam<bool>("apply_velocity_scaling");
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, getShaftConnectedUserObjectName(), params);
    connectObject(params, _junction_uo_name, "K");
  }
}

void
ShaftConnectedPump1Phase::addVariables()
{
  VolumeJunction1Phase::addVariables();

  addJunctionVariable(false, _head_var_name);
  addJunctionVariable(false, _hydraulic_torque_var_name);
  addJunctionVariable(false, _friction_torque_var_name);
  addJunctionVariable(false, _moment_of_inertia_var_name);

  if (!_app.isRestarting())
  {
    addJunctionIC(_head_var_name, 0);
    addJunctionIC(_hydraulic_torque_var_name, 0);
    addJunctionIC(_friction_torque_var_name, 0);
    addJunctionIC(_moment_of_inertia_var_name, _inertia_const);
  }
}

void
ShaftConnectedPump1Phase::addMooseObjects()
{
  VolumeJunction1Phase::addMooseObjects();

  const std::vector<std::pair<std::string, VariableName>> quantities = {
      {"pump_head", _head_var_name},
      {"hydraulic_torque", _hydraulic_torque_var_name},
      {"friction_torque", _friction_torque_var_name},
      {"moment_of_inertia", _moment_of_inertia_var_name}};
  for (const auto & quantity_and_name : quantities)
  {
    const std::string class_name =
        _use_scalar_variables ? "ShaftConnectedPump1PhaseScalarAux" : "ShaftConnectedPump1PhaseAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = quantity_and_name.second;
    params.set<MooseEnum>("quantity") = quantity_and_name.first;
    params.set<UserObjectName>("pump_uo") = getShaftConnectedUserObjectName();
    const std::string obj_name = genName(name(), quantity_and_name.first + "_aux");
    if (_use_scalar_variables)
      getTHMProblem().addAuxScalarKernel(class_name, obj_name, params);
    else
    {
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      getTHMProblem().addAuxKernel(class_name, obj_name, params);
    }
  }
}
