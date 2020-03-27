//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGeochemicalModelInterrogatorAction.h"
#include "Executioner.h"
#include "FEProblem.h"

registerMooseAction("GeochemistryApp", AddGeochemicalModelInterrogatorAction, "setup_mesh");
registerMooseAction("GeochemistryApp", AddGeochemicalModelInterrogatorAction, "init_mesh");
registerMooseAction("GeochemistryApp", AddGeochemicalModelInterrogatorAction, "create_problem");
registerMooseAction(
    "GeochemistryApp",
    AddGeochemicalModelInterrogatorAction,
    "add_distribution"); // During add_distribution, the GeochemicalModelInterroagor UserObject is
                         // added.  Since add_distribution occurs after add_userobject (see Moose.C
                         // addDependencySets) then the model_root UserObject will have been added,
                         // so getParam<UserObjectName>("model_root") will work
registerMooseAction("GeochemistryApp", AddGeochemicalModelInterrogatorAction, "setup_executioner");
registerMooseAction("GeochemistryApp", AddGeochemicalModelInterrogatorAction, "add_output");
registerMooseAction("GeochemistryApp", AddGeochemicalModelInterrogatorAction, "common_output");
registerMooseAction("GeochemistryApp",
                    AddGeochemicalModelInterrogatorAction,
                    "add_output_aux_variables");

defineLegacyParams(AddGeochemicalModelInterrogatorAction);

InputParameters
AddGeochemicalModelInterrogatorAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addRequiredParam<UserObjectName>("model_root",
                                          "The name of the GeochemicalModelRoot user object");
  params.addParam<std::vector<std::string>>(
      "swap_out_of_basis",
      "Species that should be removed from the model_root's basis and be replaced with the "
      "swap_into_basis species.  There must be the same number of species in swap_out_of_basis and "
      "swap_into_basis.  If this list contains more than one species, the swapping is performed "
      "one-by-one, starting with the first pair (swap_out_of_basis[0] and swap_into_basis[0]), "
      "then the next pair, etc");
  params.addParam<std::vector<std::string>>("swap_into_basis",
                                            "Species that should be removed from the model_root's "
                                            "equilibrium species list and added to the basis");
  params.addParam<std::vector<std::string>>(
      "activity_species",
      "Species that are provided numerical values of activity (or fugacity for gases) in the "
      "activity_value input");
  params.addParam<std::vector<Real>>("activity_values",
                                     "Numerical values for the activity (or fugacity) for the "
                                     "species in the activity_species list");
  params.addParam<unsigned int>(
      "precision",
      4,
      "Precision for printing values.  Also, if the absolute value of a stoichiometric coefficient "
      "is less than 10^(-precision) then it is set to zero");
  params.addParam<std::string>("equilibrium_species",
                               "",
                               "Only output results for this equilibrium species.  If not "
                               "provided, results for all equilibrium species will be outputted");
  MooseEnum interrogation_choice("reaction activity eqm_temperature", "reaction");
  params.addParam<MooseEnum>(
      "interrogation",
      interrogation_choice,
      "Type of interrogation to perform.  reaction: Output equilibrium species reactions and "
      "log10K.  activity: determine activity products at equilibrium.  eqm_temperature: determine "
      "temperature to ensure equilibrium");
  params.addParam<Real>("temperature",
                        25,
                        "Equilibrium constants will be computed at this temperature.  This is "
                        "ignored if interrogation=eqm_temperature.");
  params.addClassDescription("Action that sets up the geochemical model interrogator");

  return params;
}

AddGeochemicalModelInterrogatorAction::AddGeochemicalModelInterrogatorAction(InputParameters params)
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
}

void
AddGeochemicalModelInterrogatorAction::act()
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
    _problem->setKernelCoverageCheck(false);
  }
  // Add the fluid properties interrogator user object
  else if (_current_task == "add_distribution")
  {
    addGeochemicalModelInterrogatorObject();
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
  else if (_current_task == "add_output")
  {
    const std::string class_name = "Console";
    InputParameters params = _factory.getValidParams(class_name);
    params.addPrivateParam<FEProblemBase *>("_fe_problem_base", _problem.get());
    params.set<std::string>("file_base") = _app.getOutputFileBase();
    params.set<ExecFlagEnum>("execute_on") = EXEC_FINAL;
    params.set<MultiMooseEnum>("system_info") = "";
    std::shared_ptr<Output> output = _factory.create<Output>(class_name, "Console", params);
    OutputWarehouse & output_warehouse = _app.getOutputWarehouse();
    output_warehouse.addOutput(output);
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
AddGeochemicalModelInterrogatorAction::addGeochemicalModelInterrogatorObject() const
{
  const std::string class_name = "GeochemicalModelInterrogator";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("model_root") = getParam<UserObjectName>("model_root");
  // Only pass parameters that were supplied to this action
  if (isParamValid("swap_out_of_basis"))
    params.set<std::vector<std::string>>("swap_out_of_basis") =
        getParam<std::vector<std::string>>("swap_out_of_basis");
  if (isParamValid("swap_into_basis"))
    params.set<std::vector<std::string>>("swap_into_basis") =
        getParam<std::vector<std::string>>("swap_into_basis");
  if (isParamValid("activity_species"))
    params.set<std::vector<std::string>>("activity_species") =
        getParam<std::vector<std::string>>("activity_species");
  if (isParamValid("activity_values"))
    params.set<std::vector<Real>>("activity_values") =
        getParam<std::vector<Real>>("activity_values");
  params.set<unsigned int>("precision") = getParam<unsigned int>("precision");
  params.set<std::string>("equilibrium_species") = getParam<std::string>("equilibrium_species");
  params.set<MooseEnum>("interrogation") = getParam<MooseEnum>("interrogation");
  params.set<Real>("temperature") = getParam<Real>("temperature");
  _problem->addUserObject(class_name, "geochemical_model_interrogator", params);
}
