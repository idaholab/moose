//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Pump1Phase.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "Function.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", Pump1Phase);

InputParameters
Pump1Phase::validParams()
{
  InputParameters params = VolumeJunction1Phase::validParams();

  params.addRequiredParam<Real>("head", "Pump head [m]");
  params.makeParamRequired<Real>("A_ref");

  params.declareControllable("head");

  params.addClassDescription("Pump between two 1-phase flow channels that has a non-zero volume");

  return params;
}

Pump1Phase::Pump1Phase(const InputParameters & params)
  : VolumeJunction1Phase(params), _head(getParam<Real>("head"))
{
}

void
Pump1Phase::buildVolumeJunctionUserObject()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  {
    const std::string class_name = "ADPump1PhaseUserObject";
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
    params.set<Real>("head") = _head;
    params.set<Real>("gravity_magnitude") = THM::gravity_const;
    params.set<Real>("A_ref") = getParam<Real>("A_ref");
    params.set<Real>("K") = getParam<Real>("K");
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _junction_uo_name, params);
    connectObject(params, _junction_uo_name, "head");
  }
}
