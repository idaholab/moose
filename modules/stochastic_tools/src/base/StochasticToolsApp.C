//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StochasticToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
StochasticToolsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("StochasticToolsApp");

StochasticToolsApp::StochasticToolsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  StochasticToolsApp::registerAll(_factory, _action_factory, _syntax);
}

StochasticToolsApp::~StochasticToolsApp() {}

void
StochasticToolsApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"StochasticToolsApp"});
  Registry::registerActionsTo(af, {"StochasticToolsApp"});

  // Adds [Trainers] block
  registerSyntaxTask("AddSurrogateAction", "Trainers/*", "add_trainer");
  registerMooseObjectTask("add_trainer", SurrogateTrainer, false);
  addTaskDependency("add_trainer", "add_sampler");

  // Adds [Surrogates] block
  registerSyntaxTask("AddSurrogateAction", "Surrogates/*", "add_surrogate");
  registerMooseObjectTask("add_surrogate", SurrogateModel, false);
  addTaskDependency("add_surrogate", "add_trainer");

  // Adds action for loading Surrogate data
  registerTask("load_surrogate_data", true);
  addTaskDependency("load_surrogate_data", "add_surrogate");

  // Adds action for loading mapping data
  registerTask("load_mapping_data", true);
  addTaskDependency("load_mapping_data", "add_variable_mapping");

  // General StochasticTools action
  registerTask("auto_create_mesh", false);
  registerTask("auto_create_problem", false);
  registerTask("auto_create_executioner", false);
  registerSyntaxTask("StochasticToolsAction", "StochasticTools", "auto_create_mesh");
  registerSyntaxTask("StochasticToolsAction", "StochasticTools", "auto_create_problem");
  registerSyntaxTask("StochasticToolsAction", "StochasticTools", "auto_create_executioner");

  // StochasticResults
  registerTask("declare_stochastic_results_vectors", true);
  addTaskDependency("declare_stochastic_results_vectors", "add_vector_postprocessor");
  addTaskDependency("add_reporter", "declare_stochastic_results_vectors");

  // Covariance functions (Gaussian Process)
  registerSyntaxTask("AddCovarianceAction", "Covariance/*", "add_covariance");
  registerMooseObjectTask("add_covariance", CovarianceFunctionBase, false);
  addTaskDependency("add_covariance", "add_user_object");
  addTaskDependency("add_distribution", "add_covariance");
  // Mapping objects
  registerSyntaxTask("AddVariableMappingAction", "VariableMappings/*", "add_variable_mapping");
  registerMooseObjectTask("add_variable_mapping", VariableMappingBase, false);
  addTaskDependency("add_variable_mapping", "add_reporter");
  // Adds action for loading Covariance data in model
  registerTask("load_covariance_data", true);
  addTaskDependency("load_covariance_data", "load_surrogate_data");
  addTaskDependency("setup_function_complete", "load_covariance_data");
  addTaskDependency("setup_mesh", "auto_create_mesh");
  addTaskDependency("create_problem", "auto_create_problem");
  addTaskDependency("setup_executioner", "auto_create_executioner");
  // Likelihood functions (Bayesian inference)
  registerSyntaxTask("AddLikelihoodAction", "Likelihood/*", "add_likelihood");
  registerMooseObjectTask("add_likelihood", LikelihoodFunctionBase, false);
  addTaskDependency("add_likelihood", "add_user_object");
  addTaskDependency("add_distribution", "add_likelihood");

  registerSyntaxTask("AdaptiveSamplerAction", "Samplers", "add_user_object");
  registerSyntaxTask("AdaptiveSamplerAction", "Samplers", "add_postprocessor");

  // Adds [ParameterStudy] block
  registerSyntax("ParameterStudyAction", "ParameterStudy");
}

void
StochasticToolsApp::registerApps()
{
  registerApp(StochasticToolsApp);
}

void
StochasticToolsApp::requiresTorch(const MooseObject &
#ifndef MOOSE_LIBTORCH_ENABLED
                                      obj
#endif
)
{
#ifndef MOOSE_LIBTORCH_ENABLED
  obj.mooseError("PyTorch C++ API (libtorch) must be installed to use this object, see "
                 "https://mooseframework.inl.gov/modules/stochastic_tools/install_pytorch.html for "
                 "instruction.");
#endif
}

extern "C" void
StochasticToolsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  StochasticToolsApp::registerAll(f, af, s);
}
extern "C" void
StochasticToolsApp__registerApps()
{
  StochasticToolsApp::registerApps();
}
