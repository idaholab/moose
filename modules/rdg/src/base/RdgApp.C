#include "RdgApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template<>
InputParameters validParams<RdgApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

RdgApp::RdgApp(InputParameters parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RdgApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RdgApp::associateSyntax(_syntax, _action_factory);
}

RdgApp::~RdgApp()
{
}

// External entry point for dynamic application loading
extern "C" void RdgApp__registerApps() { RdgApp::registerApps(); }
void
RdgApp::registerApps()
{
  registerApp(RdgApp);
}

// External entry point for dynamic object registration
extern "C" void RdgApp__registerObjects(Factory & factory) { RdgApp::registerObjects(factory); }
void
RdgApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void RdgApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { RdgApp::associateSyntax(syntax, action_factory); }
void
RdgApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
