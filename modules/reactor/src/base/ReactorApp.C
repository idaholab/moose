#include "ReactorApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ReactorApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  return params;
}

ReactorApp::ReactorApp(InputParameters parameters) : MooseApp(parameters)
{
  ReactorApp::registerAll(_factory, _action_factory, _syntax);
}

ReactorApp::~ReactorApp() {}

void
ReactorApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"ReactorApp"});
  Registry::registerActionsTo(af, {"ReactorApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
ReactorApp::registerApps()
{
  registerApp(ReactorApp);
}

void
ReactorApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"ReactorApp"});
}

void
ReactorApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"ReactorApp"});
}

void
ReactorApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ReactorApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ReactorApp::registerAll(f, af, s);
}
extern "C" void
ReactorApp__registerApps()
{
  ReactorApp::registerApps();
}
