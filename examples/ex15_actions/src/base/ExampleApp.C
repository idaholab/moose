#include "Moose.h"
#include "ExampleApp.h"
#include "AppFactory.h"
#include "ActionFactory.h"  // <- Actions are special (they have their own factory)
#include "Syntax.h"

// Example 15 Includes
#include "Convection.h"
#include "ConvectionDiffusionAction.h"

template<>
InputParameters validParams<ExampleApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExampleApp::ExampleApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  srand(libMesh::processor_id());

  Moose::registerObjects(_factory);
  ExampleApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ExampleApp::associateSyntax(_syntax, _action_factory);
}

ExampleApp::~ExampleApp()
{
}


void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}

void
ExampleApp::registerObjects(Factory & factory)
{
  registerKernel(Convection);
}

void
ExampleApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  /**
   * Registering an Action is a little different than registering the other MOOSE
   * objects.  First, you need to register your Action in the associateSyntax method.
   * Also, you register your Action class with an "action_name" that can be
   * satisfied by executing the Action (running the "act" virtual method).
   */
  registerAction(ConvectionDiffusionAction, "add_kernel");

  /**
   * We need to tell the parser what new section name to look for and what
   * Action object to build when it finds it.  This is done directly on the syntax
   * with the registerActionSyntax method.
   *
   * The section name should be the "full path" of the parsed section but should NOT
   * contain a leading slash.  Wildcard characters can be used to replace a piece of the
   * path.
   */
  syntax.registerActionSyntax("ConvectionDiffusionAction", "ConvectionDiffusion");
}
