#include "FsiApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "NavierStokesApp.h"
#include "TensorMechanicsApp.h"

InputParameters
FsiApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  return params;
}

FsiApp::FsiApp(InputParameters parameters) : MooseApp(parameters)
{
  FsiApp::registerAll(_factory, _action_factory, _syntax);
}

FsiApp::~FsiApp() {}

void
FsiApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"FsiApp"});
  Registry::registerActionsTo(af, {"FsiApp"});

  NavierStokesApp::registerAll(f, af, s);
  TensorMechanicsApp::registerAll(f, af, s);
}

void
FsiApp::registerApps()
{
  registerApp(FsiApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FsiApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FsiApp::registerAll(f, af, s);
}
extern "C" void
FsiApp__registerApps()
{
  FsiApp::registerApps();
}
