/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "Moose.h"
#include "ExampleApp.h"
#include "AppFactory.h"
#include "ActionFactory.h" // <- Actions are special (they have their own factory)
#include "Syntax.h"
#include "MooseSyntax.h"

// Example 15 Includes
#include "ExampleConvection.h"
#include "ConvectionDiffusionAction.h"

template <>
InputParameters
validParams<ExampleApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExampleApp::ExampleApp(InputParameters parameters) : MooseApp(parameters)
{
  srand(processor_id());

  Moose::registerObjects(_factory);
  ExampleApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ExampleApp::associateSyntax(_syntax, _action_factory);
}

ExampleApp::~ExampleApp() {}

void
ExampleApp::registerApps()
{
  registerApp(ExampleApp);
}

void
ExampleApp::registerObjects(Factory & factory)
{
  registerKernel(ExampleConvection);
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
  registerSyntax("ConvectionDiffusionAction", "ConvectionDiffusion");
}
