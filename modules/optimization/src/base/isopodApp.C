#include "isopodApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

#include "IsopodAppTypes.h"

InputParameters
isopodApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

isopodApp::isopodApp(InputParameters parameters) : MooseApp(parameters)
{
  isopodApp::registerAll(_factory, _action_factory, _syntax);
}

isopodApp::~isopodApp() {}

void
isopodApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"isopodApp"});
  Registry::registerActionsTo(af, {"isopodApp"});

  auto & syntax = s;
  auto & factory = f;

  // Optimization execution flags
  registerExecFlag(EXEC_FORWARD);
  registerExecFlag(EXEC_ADJOINT);
  registerExecFlag(EXEC_HESSIAN);

  // Form Function actions
  registerSyntaxTask("AddOptimizationReporterAction", "OptimizationReporter", "add_reporter");
}

void
isopodApp::registerApps()
{
  registerApp(isopodApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
isopodApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  isopodApp::registerAll(f, af, s);
}
extern "C" void
isopodApp__registerApps()
{
  isopodApp::registerApps();
}
