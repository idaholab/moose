//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelSetup.h"
#include "MooseObjectAction.h"
#include "AddVariableAction.h"
#include "Numerics.h"

InputParameters
FlowModelSetup::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<bool>("2nd_order_mesh", false, "Use 2nd order elements in the mesh");
  params.addParam<Real>(
      "gravity_magnitude", THM::gravity_const, "Gravitational acceleration magnitude");

  return params;
}

FlowModelSetup::FlowModelSetup(const InputParameters & params)
  : _this_params(params),
    _this_app(*_this_params.getCheckedPointerParam<MooseApp *>("_moose_app")),
    _this_action_factory(_this_app.getActionFactory()),
    _this_action_warehouse(*_this_params.getCheckedPointerParam<ActionWarehouse *>("awh")),
    _fe_family(AddVariableAction::getNonlinearVariableFamilies()),
    _fe_order(AddVariableAction::getNonlinearVariableOrders()),
    _gravity_magnitude(getParam<Real>("gravity_magnitude"))
{
  if (getParam<bool>("2nd_order_mesh"))
    _fe_order = "SECOND";
}

void
FlowModelSetup::addSolutionVariable(const VariableName & var_name, const Real & scaling)
{
  const std::string class_name = "AddVariableAction";
  InputParameters params = _this_action_factory.getValidParams(class_name);
  params.set<std::vector<Real>>("scaling") = {scaling};
  params.set<MooseEnum>("family") = _fe_family;
  params.set<MooseEnum>("order") = _fe_order;

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_this_action_factory.create(class_name, var_name, params));

  _this_action_warehouse.addActionBlock(action);
}

void
FlowModelSetup::addAuxVariable(const VariableName & var_name)
{
  const std::string class_name = "AddAuxVariableAction";
  InputParameters params = _this_action_factory.getValidParams(class_name);

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_this_action_factory.create(class_name, var_name, params));

  _this_action_warehouse.addActionBlock(action);
}

void
FlowModelSetup::addFunctionIC(const VariableName & var_name, const FunctionName & function_name)
{
  const std::string class_name = "AddInitialConditionAction";
  InputParameters params = _this_action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "FunctionIC";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _this_action_factory.create(class_name, var_name + "_IC", params));

  action->getObjectParams().set<VariableName>("variable") = var_name;
  action->getObjectParams().set<FunctionName>("function") = function_name;

  _this_action_warehouse.addActionBlock(action);
}

void
FlowModelSetup::addMaterials()
{
  {
    const std::string class_name = "AddMaterialAction";
    InputParameters params = _this_action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "DirectionMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _this_action_factory.create(class_name, "direction_material", params));
    _this_action_warehouse.addActionBlock(action);
  }
}
