//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledHeatTransferAction.h"
#include "FEProblem.h"

registerMooseAction("ThermalHydraulicsApp", CoupledHeatTransferAction, "add_bc");
registerMooseAction("ThermalHydraulicsApp", CoupledHeatTransferAction, "add_user_object");
registerMooseAction("ThermalHydraulicsApp", CoupledHeatTransferAction, "add_transfer");

InputParameters
CoupledHeatTransferAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription(
      "Action that creates the necessary objects, for the solid side, to couple a "
      "solid heat conduction region to a 1-D flow channel via convective heat transfer");

  params.addRequiredParam<std::vector<BoundaryName>>("boundary",
                                                     "Boundary name(s) on the solid side");
  params.addRequiredParam<VariableName>("T", "Solid side temperature variable");
  params.addRequiredParam<VariableName>(
      "T_wall", "Variable on the flow channel side into which to transfer the solid temperature");
  params.addRequiredParam<VariableName>(
      "T_fluid", "Variable on the solid side into which to transfer the fluid temperature");
  params.addRequiredParam<VariableName>(
      "htc", "Variable on the solid side into which to transfer the heat transfer coefficient");
  MooseEnum directions("x y z");
  params.addRequiredParam<MooseEnum>("direction", directions, "The direction of the layers.");
  params.addRequiredParam<unsigned int>("num_layers", "The number of layers.");
  params.addRequiredParam<std::string>("multi_app", "The name of the multi-app.");
  params.addRequiredParam<UserObjectName>(
      "T_fluid_user_object", "Spatial user object holding the fluid temperature values");
  params.addRequiredParam<UserObjectName>(
      "htc_user_object", "Spatial user object holding the heat transfer coefficient values");

  return params;
}

CoupledHeatTransferAction::CoupledHeatTransferAction(const InputParameters & params)
  : Action(params),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _solid_temp_var_name(getParam<VariableName>("T")),
    _fluid_temp_var_name(getParam<VariableName>("T_fluid")),
    _wall_temp_var_name(getParam<VariableName>("T_wall")),
    _htc_var_name(getParam<VariableName>("htc")),
    _direction_enum(getParam<MooseEnum>("direction")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _T_avg_user_object_name(name() + "_T_avg_uo"),
    _th_T_fluid_user_object_name(getParam<UserObjectName>("T_fluid_user_object")),
    _th_htc_user_object_name(getParam<UserObjectName>("htc_user_object")),
    _multi_app_name(getParam<std::string>("multi_app"))
{
}

void
CoupledHeatTransferAction::act()
{
  if (_current_task == "add_bc")
    addBCs();
  else if (_current_task == "add_user_object")
    addUserObjects();
  else if (_current_task == "add_transfer")
    addTransfers();
}

void
CoupledHeatTransferAction::addBCs()
{
  const std::string class_name = "CoupledConvectiveHeatFluxBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = _solid_temp_var_name;
  params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
  params.set<std::vector<VariableName>>("T_infinity") = {_fluid_temp_var_name};
  params.set<std::vector<VariableName>>("htc") = {_htc_var_name};
  _problem->addBoundaryCondition(class_name, name() + "_bc", params);
}

void
CoupledHeatTransferAction::addUserObjects()
{
  const std::string class_name = "LayeredSideAverage";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<VariableName>>("variable") = {_solid_temp_var_name};
  params.set<MooseEnum>("direction") = _direction_enum;
  params.set<unsigned int>("num_layers") = _num_layers;
  params.set<std::vector<BoundaryName>>("boundary") = {_boundary};
  _problem->addUserObject(class_name, _T_avg_user_object_name, params);
}

void
CoupledHeatTransferAction::addTransfers()
{
  // Transfers to the flow channel application
  {
    const std::string class_name = "MultiAppUserObjectTransfer";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MultiAppName>("to_multi_app") = _multi_app_name;
    params.set<UserObjectName>("user_object") = {_T_avg_user_object_name};
    params.set<std::vector<AuxVariableName>>("variable") = {_wall_temp_var_name};
    _problem->addTransfer(class_name, name() + "_T_solid_transfer", params);
  }

  // Transfers from the flow channel application. Note that
  // MultiAppNearestNodeTransfer seems like it should be a more appropriate
  // choice than MultiAppUserObjectTransfer, but it has been noted that for
  // large meshes, MultiAppNearestNodeTransfer is slower than
  // MultiAppUserObjectTransfer.
  {
    const std::string class_name = "MultiAppUserObjectTransfer";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MultiAppName>("from_multi_app") = _multi_app_name;
    params.set<UserObjectName>("user_object") = {_th_T_fluid_user_object_name};
    params.set<std::vector<AuxVariableName>>("variable") = {_fluid_temp_var_name};
    _problem->addTransfer(class_name, name() + "_T_fluid_transfer", params);
  }
  {
    const std::string class_name = "MultiAppUserObjectTransfer";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MultiAppName>("from_multi_app") = _multi_app_name;
    params.set<UserObjectName>("user_object") = _th_htc_user_object_name;
    params.set<std::vector<AuxVariableName>>("variable") = {_htc_var_name};
    _problem->addTransfer(class_name, name() + "_htc_transfer", params);
  }
}
