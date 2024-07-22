//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFluidPropertiesInterrogatorAction.h"
#include "Executioner.h"
#include "FEProblem.h"

registerMooseAction("FluidPropertiesApp", AddFluidPropertiesInterrogatorAction, "setup_mesh");
registerMooseAction("FluidPropertiesApp", AddFluidPropertiesInterrogatorAction, "init_mesh");
registerMooseAction("FluidPropertiesApp", AddFluidPropertiesInterrogatorAction, "create_problem");
registerMooseAction("FluidPropertiesApp", AddFluidPropertiesInterrogatorAction, "add_user_object");
registerMooseAction("FluidPropertiesApp",
                    AddFluidPropertiesInterrogatorAction,
                    "setup_executioner");
registerMooseAction("FluidPropertiesApp", AddFluidPropertiesInterrogatorAction, "add_fp_output");
registerMooseAction("FluidPropertiesApp", AddFluidPropertiesInterrogatorAction, "common_output");
registerMooseAction("FluidPropertiesApp",
                    AddFluidPropertiesInterrogatorAction,
                    "add_output_aux_variables");

InputParameters
AddFluidPropertiesInterrogatorAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the fluid properties object to query.");
  params.addParam<Real>("rho", "Density");
  params.addParam<Real>("rhou", "Momentum density; rho * u");
  params.addParam<Real>("rhoE", "Total energy density: rho * E");
  params.addParam<Real>("e", "Specific internal energy");
  params.addParam<Real>("p", "Pressure");
  params.addParam<Real>("T", "Temperature");
  params.addParam<Real>("vel", "Velocity");
  params.addParam<std::vector<Real>>("x_ncg", "Mass fractions of NCGs");
  params.addParam<unsigned int>("precision", 10, "Precision for printing values");
  params.addParam<bool>("json", false, "Output in JSON format");

  params.addClassDescription("Action that sets up the fluid properties interrogator");

  return params;
}

AddFluidPropertiesInterrogatorAction::AddFluidPropertiesInterrogatorAction(
    const InputParameters & params)
  : Action(params)
{
  // Currently these parameters are required by the constructor of Console, which
  // assumes that the action satisfying task "add_output" has these parameters.
  // However, these parameters are not meant to be seen by the user, so they
  // are added here, instead of in validParams(), with the help of const_cast.
  InputParameters & pars = const_cast<InputParameters &>(parameters());
  ExecFlagEnum exec_enum = Output::getDefaultExecFlagEnum();
  exec_enum = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  pars.addParam<ExecFlagEnum>("execute_on", exec_enum, "(Does not need to be set)");
  pars.addParam<bool>("print_perf_log", false, "(Does not need to be set)");
  pars.addParam<bool>("print_linear_residuals", false, "(Does not need to be set)");
}

void
AddFluidPropertiesInterrogatorAction::act()
{
  // Set up an arbitrary mesh
  if (_current_task == "setup_mesh")
  {
    const std::string class_name = "GeneratedMesh";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseEnum>("dim") = "1";
    _mesh = _factory.create<MooseMesh>(class_name, "mesh", params);
  }
  // Initialize the arbitrary mesh
  else if (_current_task == "init_mesh")
  {
    _mesh->init();
  }
  // Create a "solve=false" FEProblem
  else if (_current_task == "create_problem")
  {
    const std::string class_name = "FEProblem";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseMesh *>("mesh") = _mesh.get();
    params.set<bool>("use_nonlinear") = true;
    params.set<bool>("solve") = false;
    _problem = _factory.create<FEProblemBase>(class_name, "Problem", params);
    _problem->setKernelCoverageCheck(FEProblemBase::CoverageCheckMode::FALSE);
  }
  // Add the fluid properties interrogator user object
  else if (_current_task == "add_user_object")
  {
    addFluidPropertiesInterrogatorObject();
  }
  // Set up an arbitrary steady executioner
  else if (_current_task == "setup_executioner")
  {
    const std::string class_name = "Steady";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    params.set<FEProblem *>("_fe_problem") = (std::dynamic_pointer_cast<FEProblem>(_problem)).get();
    std::shared_ptr<Executioner> executioner =
        _factory.create<Executioner>(class_name, "Executioner", params);
    _app.setExecutioner(std::move(executioner));
  }
  // Create a console that executes only on FINAL and does not print system info
  else if (_current_task == "add_fp_output")
  {
    OutputWarehouse & output_warehouse = _app.getOutputWarehouse();
    if (!output_warehouse.hasOutput("console"))
    {
      const std::string class_name = "Console";
      InputParameters params = _factory.getValidParams(class_name);
      params.addPrivateParam<FEProblemBase *>("_fe_problem_base", _problem.get());
      params.set<std::string>("file_base") = _app.getOutputFileBase();
      params.set<ExecFlagEnum>("execute_on") = EXEC_FINAL;
      params.set<MultiMooseEnum>("system_info") = "";
      std::shared_ptr<Output> output = _factory.create<Output>(class_name, "Console", params);
      output_warehouse.addOutput(output);
    }
  }
  else if (_current_task == "common_output")
  {
    // This action must satisfy this task to prevent CommonOutputAction from
    // acting, which performs a static cast that assumes that the action
    // satisfying the task "add_output" is derived from MooseObjectAction,
    // which is now false.
  }
  else if (_current_task == "add_output_aux_variables")
  {
    // This action must satisfy this task to prevent MaterialOutputAction from
    // acting, which assumes that the action satisfying "add_output" can be
    // dynamic_cast-ed to type "AddOutputAction", which is now false.
  }
}

void
AddFluidPropertiesInterrogatorAction::addFluidPropertiesInterrogatorObject() const
{
  const std::string class_name = "FluidPropertiesInterrogator";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");
  // Only pass parameters that were supplied to this action
  if (isParamValid("rho"))
    params.set<Real>("rho") = getParam<Real>("rho");
  if (isParamValid("rhou"))
    params.set<Real>("rhou") = getParam<Real>("rhou");
  if (isParamValid("rhoE"))
    params.set<Real>("rhoE") = getParam<Real>("rhoE");
  if (isParamValid("e"))
    params.set<Real>("e") = getParam<Real>("e");
  if (isParamValid("p"))
    params.set<Real>("p") = getParam<Real>("p");
  if (isParamValid("T"))
    params.set<Real>("T") = getParam<Real>("T");
  if (isParamValid("vel"))
    params.set<Real>("vel") = getParam<Real>("vel");
  if (isParamValid("x_ncg"))
    params.set<std::vector<Real>>("x_ncg") = getParam<std::vector<Real>>("x_ncg");
  params.set<unsigned int>("precision") = getParam<unsigned int>("precision");
  params.set<bool>("json") = getParam<bool>("json");
  _problem->addUserObject(class_name, "fp_interrogator", params);
}
