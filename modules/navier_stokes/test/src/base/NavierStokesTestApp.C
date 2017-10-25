#include "NavierStokesTestApp.h"
#include "NavierStokesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

// Scalar advection equation stabilized by SUPG.
#include "Advection.h"

template <>
InputParameters
validParams<NavierStokesTestApp>()
{
  InputParameters params = validParams<NavierStokesApp>();
  return params;
}

NavierStokesTestApp::NavierStokesTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  NavierStokesApp::registerObjectDepends(_factory);
  NavierStokesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  NavierStokesApp::associateSyntaxDepends(_syntax, _action_factory);
  NavierStokesApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    NavierStokesTestApp::registerObjects(_factory);
    NavierStokesTestApp::associateSyntax(_syntax, _action_factory);
  }
}

NavierStokesTestApp::~NavierStokesTestApp() {}

// External entry point for dynamic application loading
extern "C" void
NavierStokesTestApp__registerApps()
{
  NavierStokesTestApp::registerApps();
}
void
NavierStokesTestApp::registerApps()
{
  registerApp(NavierStokesApp);
  registerApp(NavierStokesTestApp);
}

// External entry point for dynamic object registration
extern "C" void
NavierStokesTestApp__registerObjects(Factory & factory)
{
  NavierStokesTestApp::registerObjects(factory);
}
void
NavierStokesTestApp::registerObjects(Factory & factory)
{
  // Scalar advection equation stabilized by SUPG.
  registerKernel(Advection);
}

// External entry point for dynamic syntax association
extern "C" void
NavierStokesTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  NavierStokesTestApp::associateSyntax(syntax, action_factory);
}
void
NavierStokesTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
