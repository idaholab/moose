#include "StochasticToolsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template<>
InputParameters validParams<StochasticToolsApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

StochasticToolsApp::StochasticToolsApp(InputParameters parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  StochasticToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  StochasticToolsApp::associateSyntax(_syntax, _action_factory);
}

StochasticToolsApp::~StochasticToolsApp()
{
}

// External entry point for dynamic application loading
extern "C" void StochasticToolsApp__registerApps() { StochasticToolsApp::registerApps(); }
void
StochasticToolsApp::registerApps()
{
  registerApp(StochasticToolsApp);
}

// External entry point for dynamic object registration
extern "C" void StochasticToolsApp__registerObjects(Factory & factory) { StochasticToolsApp::registerObjects(factory); }
void
StochasticToolsApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void StochasticToolsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { StochasticToolsApp::associateSyntax(syntax, action_factory); }
void
StochasticToolsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
