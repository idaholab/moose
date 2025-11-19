#include "ShiftedBoundaryMethodApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ShiftedBoundaryMethodApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

ShiftedBoundaryMethodApp::ShiftedBoundaryMethodApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  ShiftedBoundaryMethodApp::registerAll(_factory, _action_factory, _syntax);
}

ShiftedBoundaryMethodApp::~ShiftedBoundaryMethodApp() {}

void
ShiftedBoundaryMethodApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"ShiftedBoundaryMethodApp"});
  Registry::registerActionsTo(af, {"ShiftedBoundaryMethodApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
ShiftedBoundaryMethodApp::registerApps()
{
  registerApp(ShiftedBoundaryMethodApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ShiftedBoundaryMethodApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ShiftedBoundaryMethodApp::registerAll(f, af, s);
}
extern "C" void
ShiftedBoundaryMethodApp__registerApps()
{
  ShiftedBoundaryMethodApp::registerApps();
}
