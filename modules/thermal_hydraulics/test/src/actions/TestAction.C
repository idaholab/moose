//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "FEProblemBase.h"

InputParameters
TestAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addParam<bool>("use_transient_executioner", "Option to use a transient executioner");
  params.addParam<bool>("generate_mesh", true, "Option to have the action generate the mesh");
  params.addParam<std::vector<VariableName>>(
      "scalar_variable_names", {}, "List of scalar variables");
  params.addParam<std::vector<FunctionName>>(
      "scalar_variable_values", {}, "List of values of the scalar variables");
  params.addParam<std::vector<VariableName>>("aux_variable_names", {}, "List of aux variables");
  params.addParam<std::vector<FunctionName>>(
      "aux_variable_values", {}, "List of values of the aux variables");
  params.addParam<std::vector<std::string>>(
      "mat_property_names", {}, "List of material property names");
  params.addParam<std::vector<FunctionName>>(
      "mat_property_values", {}, "List of values of the material properties");
  params.addParam<bool>("ad", false, "Setup for AD or non-AD testing");

  params.addPrivateParam<std::string>("fe_family");
  params.addPrivateParam<std::string>("fe_order");

  params.addParam<bool>("abort_on_solve_fail", false, "Abort if the solve did not converge rather than cut the timestep");

  return params;
}

TestAction::TestAction(const InputParameters & params)
  : Action(params),
    // if a derived class should have a different default, then that class needs
    // to set this parameter in its constructor
    _default_use_transient_executioner(false),

    _scalar_variables(getParam<std::vector<VariableName>>("scalar_variable_names")),
    _scalar_variable_values(getParam<std::vector<FunctionName>>("scalar_variable_values")),
    _aux_variables(getParam<std::vector<VariableName>>("aux_variable_names")),
    _aux_variable_values(getParam<std::vector<FunctionName>>("aux_variable_values")),
    _mat_property_names(getParam<std::vector<std::string>>("mat_property_names")),
    _mat_property_values(getParam<std::vector<FunctionName>>("mat_property_values")),
    _fe_family(getParam<std::string>("fe_family")),
    _fe_order(getParam<std::string>("fe_order")),
    _ad(getParam<bool>("ad"))
{
  if (_scalar_variables.size() != _scalar_variable_values.size())
    mooseError(name(),
               ": The parameters 'scalar_variable_names' and ",
               "'scalar_variable_values' must have the same numbers of entries.");
  if (_aux_variables.size() != _aux_variable_values.size())
    mooseError(name(),
               ": The parameters 'aux_variable_names' and ",
               "'aux_variable_values' must have the same numbers of entries.");
  if (_mat_property_names.size() != _mat_property_values.size())
    mooseError(name(),
               ": The parameters 'mat_property_names' and ",
               "'mat_property_values' must have the same numbers of entries.");
}

void
TestAction::act()
{
  if (_current_task == "meta_action")
  {
    if (getParam<bool>("generate_mesh"))
      addMesh();

    addObjects();

    {
      const std::string class_name = "CreateProblemAction";
      auto action_params = _action_factory.getValidParams(class_name);
      action_params.set<std::string>("type") = "FEProblem";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, "fe_problem", action_params));

      action->getObjectParams().set<MooseEnum>("kernel_coverage_check") = "false";
      _awh.addActionBlock(action);
    }
  }
}

void
TestAction::addMesh()
{
  addMeshInternal(1);
}

void
TestAction::addMeshInternal(const unsigned int & nx)
{
  std::vector<std::string> setup_mesh_tasks{"setup_mesh", "set_mesh_base", "init_mesh"};
  for (const std::string & task : setup_mesh_tasks)
  {
    const std::string class_name = "SetupMeshAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "GeneratedMesh";
    params.set<std::string>("task") = task;

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, task, params));

    action->getObjectParams().set<MooseEnum>("dim") = 1;
    action->getObjectParams().set<unsigned int>("nx") = nx;
    action->getObjectParams().set<bool>("allow_renumbering") = false;

    _awh.addActionBlock(action);
  }

  std::vector<std::string> setup_mesh_complete_tasks{"prepare_mesh", "setup_mesh_complete"};
  for (const std::string & task : setup_mesh_complete_tasks)
  {
    const std::string class_name = "SetupMeshCompleteAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("task") = task;

    std::shared_ptr<Action> action =
        std::static_pointer_cast<Action>(_action_factory.create(class_name, task, params));

    _awh.addActionBlock(action);
  }
}

void
TestAction::addObjects()
{
  addSolutionVariables();
  addScalarVariables(_scalar_variables, _scalar_variable_values);
  addAuxVariables();
  addInitialConditions();
  addUserObjects();
  addMaterials();
  addPreconditioner();
  addExecutioner();
  addOutput();
}

void
TestAction::addScalarVariables(const std::vector<VariableName> & names,
                               const std::vector<FunctionName> & values)
{
  for (unsigned int i = 0; i < names.size(); ++i)
  {
    // add the scalar variable
    addSolutionVariable(names[i], "SCALAR", "FIRST");

    // add its IC
    {
      const std::string class_name = "AddInitialConditionAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "FunctionScalarIC";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, names[i] + "_IC", params));

      action->getObjectParams().set<VariableName>("variable") = names[i];
      action->getObjectParams().set<std::vector<FunctionName>>("function") = {values[i]};

      _awh.addActionBlock(action);
    }
  }
}

void
TestAction::addAuxVariables()
{
  const std::vector<VariableName> names = _aux_variables;
  const std::vector<FunctionName> values = _aux_variable_values;
  const std::string fe_family = _fe_family;
  const std::string fe_order = _fe_order;

  for (unsigned int i = 0; i < names.size(); ++i)
  {
    // add the aux variable
    addAuxVariable(names[i], fe_family, fe_order);

    // add its IC
    addFunctionIC(names[i], values[i]);

    // add its aux kernel
    {
      const std::string class_name = "AddKernelAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "FunctionAux";
      params.set<std::string>("task") = "add_aux_kernel";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, names[i] + "_aux", params));

      action->getObjectParams().set<AuxVariableName>("variable") = names[i];
      action->getObjectParams().set<FunctionName>("function") = values[i];

      _awh.addActionBlock(action);
    }
  }
}

void
TestAction::addMaterials()
{
  const std::string class_name = "AddMaterialAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "GenericFunctionMaterial";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, "material", params));

  action->getObjectParams().set<std::vector<std::string>>("prop_names") = _mat_property_names;
  action->getObjectParams().set<std::vector<FunctionName>>("prop_values") = _mat_property_values;

  _awh.addActionBlock(action);
}

void
TestAction::addSolutionVariable(const VariableName & var_name,
                                const std::string & family,
                                const std::string & order,
                                const Real & scaling)
{
  const std::string class_name = "AddVariableAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<MooseEnum>("family") = family;
  params.set<MooseEnum>("order") = order;
  params.set<std::vector<Real>>("scaling") = {scaling};

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_action_factory.create(class_name, var_name, params));

  _awh.addActionBlock(action);
}

void
TestAction::addAuxVariable(const VariableName & var_name,
                           const std::string & fe_family,
                           const std::string & fe_order)
{
  const std::string class_name = "AddAuxVariableAction";
  InputParameters params = _action_factory.getValidParams(class_name);

  params.set<MooseEnum>("family") = fe_family;
  params.set<MooseEnum>("order") = fe_order;

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_action_factory.create(class_name, var_name, params));

  _awh.addActionBlock(action);
}

void
TestAction::addConstantIC(const VariableName & var_name, const Real & value)
{
  const std::string class_name = "AddInitialConditionAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "ConstantIC";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, var_name + "_IC", params));

  action->getObjectParams().set<VariableName>("variable") = var_name;
  action->getObjectParams().set<Real>("value") = value;

  _awh.addActionBlock(action);
}

void
TestAction::addFunctionIC(const VariableName & var_name, const FunctionName & function_name)
{
  const std::string class_name = "AddInitialConditionAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "FunctionIC";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, var_name + "_IC", params));

  action->getObjectParams().set<VariableName>("variable") = var_name;
  action->getObjectParams().set<FunctionName>("function") = function_name;

  _awh.addActionBlock(action);
}

void
TestAction::addPreconditioner()
{
}

void
TestAction::addExecutioner()
{
  const std::string class_name = "CreateExecutionerAction";

  // determine whether to use a transient executioner - different derived
  // classes have different defaults: the declared default is irrelevant
  bool use_transient_executioner;
  if (isParamValid("use_transient_executioner"))
    use_transient_executioner = getParam<bool>("use_transient_executioner");
  else
    use_transient_executioner = _default_use_transient_executioner;

  // Due to more consistent divergence status reporting in PETSc (as of 5f3c5e7a), users should have
  // the option to abort on the first fail if desired. Otherwise Jacobian testing, for example, could
  // fail in undesired ways, even if the Jacobian test achieves a passing result.
  bool abort_on_solve_fail = getParam<bool>("abort_on_solve_fail");

  // if a time kernel is being tested, then use a transient executioner instead of steady
  if (use_transient_executioner)
  {
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "Transient";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "executioner", params));

    action->getObjectParams().set<unsigned int>("num_steps") = 1;

    if (abort_on_solve_fail)
      action->getObjectParams().set<bool>("abort_on_solve_fail") = abort_on_solve_fail;

    _awh.addActionBlock(action);
  }
  else
  {
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "Steady";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "executioner", params));

    if (abort_on_solve_fail)
      action->getObjectParams().set<bool>("abort_on_solve_fail") = abort_on_solve_fail;

    _awh.addActionBlock(action);
  }
}

void
TestAction::addOutput()
{
}
