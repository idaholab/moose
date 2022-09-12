//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JacobianTestGeneralAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "MooseEnum.h"

registerMooseAction("ThermalHydraulicsTestApp", JacobianTestGeneralAction, "meta_action");

InputParameters
JacobianTestGeneralAction::validParams()
{
  InputParameters params = JacobianTestAction::validParams();

  params.addParam<std::vector<VariableName>>("variable_names", "List of variables");
  params.addParam<std::vector<FunctionName>>("variable_values", "List of values of the variables");

  params.set<std::string>("fe_family") = "LAGRANGE";
  params.set<std::string>("fe_order") = "FIRST";

  return params;
}

JacobianTestGeneralAction::JacobianTestGeneralAction(const InputParameters & params)
  : JacobianTestAction(params),
    _variables(getParam<std::vector<VariableName>>("variable_names")),
    _variable_values(getParam<std::vector<FunctionName>>("variable_values"))
{
  if (_variables.size() != _variable_values.size())
    mooseError("JacobianTestGeneral: The list parameters 'variable_names' and ",
               "'variable_values' must have the same length.");
}

void
JacobianTestGeneralAction::addInitialConditions()
{
}

void
JacobianTestGeneralAction::addSolutionVariables()
{
  for (unsigned int i = 0; i < _variables.size(); ++i)
  {
    addSolutionVariable(_variables[i]);
    addFunctionIC(_variables[i], _variable_values[i]);
  }
}

void
JacobianTestGeneralAction::addAuxVariables()
{
  TestAction::addAuxVariables();
}

void
JacobianTestGeneralAction::addMaterials()
{
  TestAction::addMaterials();
  // unity
  {
    const std::string class_name = "AddMaterialAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "ConstantMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "unity_material", params));

    action->getObjectParams().set<std::string>("property_name") = "unity";
    action->getObjectParams().set<Real>("value") = 1.0;
    action->getObjectParams().set<std::vector<VariableName>>("derivative_vars") = _variables;

    _awh.addActionBlock(action);
  }
}

void
JacobianTestGeneralAction::addUserObjects()
{
}
