//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ClosureTestAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "MooseObjectAction.h"

InputParameters
ClosureTestAction::validParams()
{
  InputParameters params = TestAction::validParams();

  params.addParam<FunctionName>("T_wall", "Wall temperature function");
  params.addParam<Real>("q_wall", 0., "Convective wall heat flux");
  params.addParam<std::vector<std::string>>("output", "List of material properties to output");
  params.addParam<std::vector<std::string>>("ad_output",
                                            "List of AD material properties to output");

  params.set<std::string>("fe_family") = "LAGRANGE";
  params.set<std::string>("fe_order") = "FIRST";

  return params;
}

ClosureTestAction::ClosureTestAction(const InputParameters & params)
  : TestAction(params),
    _dummy_name("dummy"),
    _T_wall_name("T_wall"),
    _has_T_wall(isParamValid("T_wall")),
    _T_wall_fn(_has_T_wall ? getParam<FunctionName>("T_wall") : ""),
    _has_q_wall(isParamValid("q_wall")),
    _q_wall(getParam<Real>("q_wall")),
    _output_properties(getParam<std::vector<std::string>>("output")),
    _output_ad_properties(getParam<std::vector<std::string>>("ad_output"))
{
  _default_use_transient_executioner = true;
}

void
ClosureTestAction::addInitialConditions()
{
  if (_has_T_wall)
    addFunctionIC(_T_wall_name, _T_wall_fn);
}

void
ClosureTestAction::addSolutionVariables()
{
  addSolutionVariable(_dummy_name);
}

void
ClosureTestAction::addAuxVariables()
{
  TestAction::addAuxVariables();
  if (_has_T_wall)
    addAuxVariable(_T_wall_name, _fe_family, _fe_order);
}

void
ClosureTestAction::addMaterials()
{
  TestAction::addMaterials();
  if (_has_q_wall)
  {
    const std::string class_name = "AddMaterialAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    if (_ad)
      params.set<std::string>("type") = "ADGenericConstantMaterial";
    else
      params.set<std::string>("type") = "GenericConstantMaterial";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "q_wall_mat", params));

    action->getObjectParams().set<std::vector<std::string>>("prop_names") = {"q_wall"};
    action->getObjectParams().set<std::vector<Real>>("prop_values") = {_q_wall};

    _awh.addActionBlock(action);
  }
}

void
ClosureTestAction::addOutput()
{
  setupOutput();
  setupADOutput();

  if ((_output_properties.size() > 0) || (_output_ad_properties.size() > 0))
  {
    const std::string class_name = "AddOutputAction";
    InputParameters action_params = _action_factory.getValidParams(class_name);
    action_params.set<std::string>("type") = "CSV";

    auto action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "csv", action_params));

    ExecFlagEnum execute_options(MooseUtils::getDefaultExecFlagEnum());
    execute_options = EXEC_TIMESTEP_END;
    action->getObjectParams().set<ExecFlagEnum>("execute_on") = execute_options;

    _awh.addActionBlock(action);
  }
}

void
ClosureTestAction::setupOutput()
{
  for (auto & prop : _output_properties)
  {
    addAuxVariable(prop, "MONOMIAL", "CONSTANT");

    {
      const std::string class_name = "AddKernelAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "MaterialRealAux";
      params.set<std::string>("task") = "add_aux_kernel";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, prop + "_aux", params));

      action->getObjectParams().set<AuxVariableName>("variable") = prop;
      action->getObjectParams().set<MaterialPropertyName>("property") = prop;

      _awh.addActionBlock(action);
    }

    {
      const std::string class_name = "AddPostprocessorAction";
      InputParameters action_params = _action_factory.getValidParams(class_name);
      action_params.set<std::string>("type") = "ElementalVariableValue";

      auto action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, prop, action_params));

      action->getObjectParams().set<VariableName>("variable") = prop;
      action->getObjectParams().set<unsigned int>("elementid") = 0;

      _awh.addActionBlock(action);
    }
  }
}

void
ClosureTestAction::setupADOutput()
{
  for (auto & prop : _output_ad_properties)
  {
    addAuxVariable(prop, "MONOMIAL", "CONSTANT");

    {
      const std::string class_name = "AddKernelAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "ADMaterialRealAux";
      params.set<std::string>("task") = "add_aux_kernel";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, prop + "_aux", params));

      action->getObjectParams().set<AuxVariableName>("variable") = prop;
      action->getObjectParams().set<MaterialPropertyName>("property") = prop;

      _awh.addActionBlock(action);
    }

    {
      const std::string class_name = "AddPostprocessorAction";
      InputParameters action_params = _action_factory.getValidParams(class_name);
      action_params.set<std::string>("type") = "ElementalVariableValue";

      auto action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, prop, action_params));

      action->getObjectParams().set<VariableName>("variable") = prop;
      action->getObjectParams().set<unsigned int>("elementid") = 0;

      _awh.addActionBlock(action);
    }
  }
}
