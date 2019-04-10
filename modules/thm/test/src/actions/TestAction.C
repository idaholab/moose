#include "TestAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "FEProblemBase.h"

template <>
InputParameters
validParams<TestAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<bool>("use_transient_executioner", "Option to use a transient executioner");
  params.addParam<bool>("generate_mesh", true, "Option to have the action generate the mesh");
  params.addParam<std::vector<VariableName>>("scalar_variable_names", "List of scalar variables");
  params.addParam<std::vector<Real>>("scalar_variable_values",
                                     "List of values of the scalar variables");
  params.addParam<std::vector<VariableName>>("constant_aux_variable_names",
                                             "List of constant aux variables");
  params.addParam<std::vector<Real>>("constant_aux_variable_values",
                                     "List of values of the constant aux variables");
  params.addParam<std::vector<std::string>>("constant_mat_property_names",
                                            "List of constant material property names");
  params.addParam<std::vector<Real>>("constant_mat_property_values",
                                     "List of values of the constant material properties");

  params.addPrivateParam<std::string>("fe_family");
  params.addPrivateParam<std::string>("fe_order");

  return params;
}

TestAction::TestAction(InputParameters params)
  : Action(params),
    // if a derived class should have a different default, then that class needs
    // to set this parameter in its constructor
    _default_use_transient_executioner(false),

    _scalar_variables(getParam<std::vector<VariableName>>("scalar_variable_names")),
    _scalar_variable_values(getParam<std::vector<Real>>("scalar_variable_values")),
    _constant_aux_variables(getParam<std::vector<VariableName>>("constant_aux_variable_names")),
    _constant_aux_variable_values(getParam<std::vector<Real>>("constant_aux_variable_values")),
    _constant_mat_property_names(getParam<std::vector<std::string>>("constant_mat_property_names")),
    _constant_mat_property_values(getParam<std::vector<Real>>("constant_mat_property_values")),
    _fe_family(getParam<std::string>("fe_family")),
    _fe_order(getParam<std::string>("fe_order"))
{
  if (_scalar_variables.size() != _scalar_variable_values.size())
    mooseError(name(),
               ": The parameters 'scalar_variable_names' and ",
               "'scalar_variable_values' must have the same numbers of entries.");
  if (_constant_aux_variables.size() != _constant_aux_variable_values.size())
    mooseError(name(),
               ": The parameters 'constant_aux_variable_names' and ",
               "'constant_aux_variable_values' must have the same numbers of entries.");
  if (_constant_mat_property_names.size() != _constant_mat_property_values.size())
    mooseError(name(),
               ": The parameters 'constant_mat_property_names' and ",
               "'constant_mat_property_values' must have the same numbers of entries.");
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

      action->getObjectParams().set<bool>("kernel_coverage_check") = false;
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
  addConstantAuxVariables(_fe_family, _fe_order);
  addNonConstantAuxVariables();
  addInitialConditions();
  addUserObjects();
  addConstantMaterials();
  addMaterials();
  addPreconditioner();
  addExecutioner();
  addOutput();
}

void
TestAction::addScalarVariables(const std::vector<VariableName> & names,
                               const std::vector<Real> & values)
{
  for (unsigned int i = 0; i < names.size(); ++i)
  {
    // add the scalar variable
    addSolutionVariable(names[i], "SCALAR", "FIRST");

    // add its IC
    {
      const std::string class_name = "AddInitialConditionAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "ScalarConstantIC";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, names[i] + "_IC", params));

      action->getObjectParams().set<VariableName>("variable") = names[i];
      action->getObjectParams().set<Real>("value") = values[i];

      _awh.addActionBlock(action);
    }
  }
}

void
TestAction::addConstantAuxVariables(const std::string & fe_family, const std::string & fe_order)
{
  addConstantAuxVariables(
      _constant_aux_variables, _constant_aux_variable_values, fe_family, fe_order);
}

void
TestAction::addConstantAuxVariables(const std::vector<VariableName> & names,
                                    const std::vector<Real> & values,
                                    const std::string & fe_family,
                                    const std::string & fe_order)
{
  for (unsigned int i = 0; i < names.size(); ++i)
  {
    // add the aux variable
    addAuxVariable(names[i], fe_family, fe_order);

    // add its IC
    {
      const std::string class_name = "AddInitialConditionAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "ConstantIC";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, names[i] + "_IC", params));

      action->getObjectParams().set<VariableName>("variable") = names[i];
      action->getObjectParams().set<Real>("value") = values[i];

      _awh.addActionBlock(action);
    }

    // add its aux kernel
    {
      const std::string class_name = "AddKernelAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "ConstantAux";
      params.set<std::string>("task") = "add_aux_kernel";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, names[i] + "_aux", params));

      action->getObjectParams().set<AuxVariableName>("variable") = names[i];
      action->getObjectParams().set<Real>("value") = values[i];

      _awh.addActionBlock(action);
    }
  }
}

void
TestAction::addConstantMaterials()
{
  const std::string class_name = "AddMaterialAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "GenericConstantMaterial";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, "constant_material", params));

  action->getObjectParams().set<std::vector<std::string>>("prop_names") =
      _constant_mat_property_names;
  action->getObjectParams().set<std::vector<Real>>("prop_values") = _constant_mat_property_values;

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
  params.set<Real>("scaling") = scaling;

  std::shared_ptr<Action> action =
      std::static_pointer_cast<Action>(_action_factory.create(class_name, var_name, params));

  _awh.addActionBlock(action);
}

void
TestAction::addConstantSolutionVariables(const std::vector<VariableName> & names,
                                         const std::vector<Real> & values,
                                         const std::string & family,
                                         const std::string & order)
{
  for (unsigned int i = 0; i < names.size(); ++i)
  {
    // add the variable
    addSolutionVariable(names[i], family, order);

    // add its IC
    {
      const std::string class_name = "AddInitialConditionAction";
      InputParameters params = _action_factory.getValidParams(class_name);
      params.set<std::string>("type") = "ConstantIC";

      std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, names[i] + "_IC", params));

      action->getObjectParams().set<VariableName>("variable") = names[i];
      action->getObjectParams().set<Real>("value") = values[i];

      _awh.addActionBlock(action);
    }
  }
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

  // if a time kernel is being tested, then use a transient executioner instead of steady
  if (use_transient_executioner)
  {
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "Transient";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "executioner", params));

    action->getObjectParams().set<unsigned int>("num_steps") = 1;

    _awh.addActionBlock(action);
  }
  else
  {
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<std::string>("type") = "Steady";

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(class_name, "executioner", params));

    _awh.addActionBlock(action);
  }
}

void
TestAction::addOutput()
{
}
