//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedTurbine1Phase.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"
#include "Shaft.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"
#include "ScalarKernel.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedTurbine1Phase);

InputParameters
ShaftConnectedTurbine1Phase::validParams()
{
  InputParameters params = VolumeJunction1Phase::validParams();
  params += ShaftConnectable::validParams();
  params.makeParamRequired<Real>("A_ref");
  params.addRequiredParam<BoundaryName>("inlet", "Turbine inlet");
  params.addRequiredParam<BoundaryName>("outlet", "Turbine outlet");
  params.suppressParameter<std::vector<BoundaryName>>("connections");
  params.addRequiredParam<Real>("omega_rated", "Rated turbine speed [rad/s]");
  params.addRequiredParam<Real>("D_wheel",
                                "Any physical dimension of the turbine as a measure of the "
                                "turbine's size, usually the rotor diameter [m]");
  params.addRequiredParam<Real>("speed_cr_fr", "Turbine speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Turbine friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Turbine friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Turbine speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Turbine inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff",
                                             "Turbine inertia coefficients [kg-m^2]");
  params.addRequiredParam<FunctionName>("head_coefficient",
                                        "Head coefficient vs flow coefficient function [-]");
  params.addRequiredParam<FunctionName>("power_coefficient",
                                        "Power coefficient vs flow coefficient function [-]");

  params.addClassDescription(
      "1-phase turbine that must be connected to a Shaft component. Turbine speed "
      "is controlled by the connected shaft; Driving torque and delta_p are "
      "computed by user input functions of inlet flow rate (flow coefficient aux variable) and "
      "shaft speed");

  return params;
}

ShaftConnectedTurbine1Phase::ShaftConnectedTurbine1Phase(const InputParameters & parameters)
  : VolumeJunction1Phase(parameters),
    ShaftConnectable(this),
    _inlet(getParam<BoundaryName>("inlet")),
    _outlet(getParam<BoundaryName>("outlet")),
    _omega_rated(getParam<Real>("omega_rated")),
    _D_wheel(getParam<Real>("D_wheel")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _head_coefficient(getParam<FunctionName>("head_coefficient")),
    _power_coefficient(getParam<FunctionName>("power_coefficient")),
    _delta_p_var_name(genName(name(), "delta_p")),
    _power_var_name(genName(name(), "power")),
    _driving_torque_var_name(genName(name(), "driving_torque")),
    _friction_torque_var_name(genName(name(), "friction_torque")),
    _flow_coeff_var_name(genName(name(), "flow_coeff")),
    _moi_var_name(genName(name(), "moment_of_inertia"))
{
  // this determines connection ordering
  addConnection(_inlet);
  addConnection(_outlet);

  checkSizeEqualsValue<Real>("tau_fr_coeff", 4);
  checkSizeEqualsValue<Real>("inertia_coeff", 4);
}

void
ShaftConnectedTurbine1Phase::check() const
{
  VolumeJunction1Phase::check();
  checkShaftConnection(this);
}

void
ShaftConnectedTurbine1Phase::buildVolumeJunctionUserObject()
{
  const Component & c = getComponentByName<Component>(_shaft_name);
  const Shaft & scc = dynamic_cast<const Shaft &>(c);
  const VariableName omega_var_name = scc.getOmegaVariableName();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADShaftConnectedTurbine1PhaseUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = _boundary_names;
    params.set<std::vector<Real>>("normals") = _normals;
    params.set<std::vector<processor_id_type>>("processor_ids") = _proc_ids;
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
    params.set<Real>("D_wheel") = _D_wheel;
    params.set<Real>("speed_cr_fr") = _speed_cr_fr;
    params.set<Real>("tau_fr_const") = _tau_fr_const;
    params.set<std::vector<Real>>("tau_fr_coeff") = _tau_fr_coeff;
    params.set<Real>("speed_cr_I") = _speed_cr_I;
    params.set<Real>("inertia_const") = _inertia_const;
    params.set<std::vector<Real>>("inertia_coeff") = _inertia_coeff;
    params.set<FunctionName>("head_coefficient") = _head_coefficient;
    params.set<FunctionName>("power_coefficient") = _power_coefficient;
    params.set<std::vector<VariableName>>("omega") = {omega_var_name};
    params.set<Real>("A_ref") = getParam<Real>("A_ref");
    params.set<Real>("K") = getParam<Real>("K");
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::string>("turbine_name") = cname();
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, getShaftConnectedUserObjectName(), params);
  }
}

void
ShaftConnectedTurbine1Phase::addVariables()
{
  VolumeJunction1Phase::addVariables();

  getTHMProblem().addSimVariable(false, _delta_p_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_delta_p_var_name, 0);

  getTHMProblem().addSimVariable(false, _power_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_power_var_name, 0);

  getTHMProblem().addSimVariable(false, _driving_torque_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_driving_torque_var_name, 0);

  getTHMProblem().addSimVariable(false, _flow_coeff_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_flow_coeff_var_name, 0);

  getTHMProblem().addSimVariable(false, _friction_torque_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_friction_torque_var_name, 0);

  getTHMProblem().addSimVariable(false, _moment_of_inertia_var_name, FEType(FIRST, SCALAR));
  getTHMProblem().addConstantScalarIC(_moment_of_inertia_var_name, _inertia_const);
}

void
ShaftConnectedTurbine1Phase::addMooseObjects()
{
  VolumeJunction1Phase::addMooseObjects();

  {
    std::string class_name = "Turbine1PhaseDeltaPAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _delta_p_var_name;
    params.set<UserObjectName>("turbine_uo") = getShaftConnectedUserObjectName();

    getTHMProblem().addAuxScalarKernel(class_name, genName(name(), "delta_p_aux"), params);
  }
  {
    std::string class_name = "Turbine1PhasePowerAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _power_var_name;
    params.set<UserObjectName>("turbine_uo") = getShaftConnectedUserObjectName();

    getTHMProblem().addAuxScalarKernel(class_name, genName(name(), "power"), params);
  }
  {
    std::string class_name = "Turbine1PhaseDrivingTorqueAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _driving_torque_var_name;
    params.set<UserObjectName>("turbine_uo") = getShaftConnectedUserObjectName();

    getTHMProblem().addAuxScalarKernel(class_name, genName(name(), "driving_torque_aux"), params);
  }
  {
    std::string class_name = "Turbine1PhaseFlowCoefficientAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _flow_coeff_var_name;
    params.set<UserObjectName>("turbine_uo") = getShaftConnectedUserObjectName();

    getTHMProblem().addAuxScalarKernel(class_name, genName(name(), "flow_coeff_aux"), params);
  }
  {
    std::string class_name = "Turbine1PhaseFrictionTorqueAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _friction_torque_var_name;
    params.set<UserObjectName>("turbine_uo") = getShaftConnectedUserObjectName();

    getTHMProblem().addAuxScalarKernel(class_name, genName(name(), "friction_torque_aux"), params);
  }
  {
    std::string class_name = "Turbine1PhaseMomentOfInertiaAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _moment_of_inertia_var_name;
    params.set<UserObjectName>("turbine_uo") = getShaftConnectedUserObjectName();

    getTHMProblem().addAuxScalarKernel(class_name, genName(name(), "inertia_aux"), params);
  }
}
