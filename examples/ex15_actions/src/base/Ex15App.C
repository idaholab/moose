#include "Ex15App.h"
#include "Factory.h"
#include "ActionFactory.h"  // <- Actions are special (they have their own factory)

// Example 15 Includes
#include "Convection.h"
#include "ConvectionDiffusionAction.h"

Ex15App::Ex15App(int argc, char *argv[]) :
    MooseApp(argc, argv)
{
  init();

  Ex15App::registerObjects();
  Ex15App::associateSyntax(_syntax);
}

void
Ex15App::registerObjects()
{
  registerKernel(Convection);

  /**
   * Registering an Action is a little different than registering the other MOOSE
   * objects.  You register your Action class with an "action_name" that can be
   * satisfied by executing the Action (running the "act" virtual method).
   */
  registerAction(ConvectionDiffusionAction, "add_kernel");
}

void
Ex15App::associateSyntax(Syntax & syntax)
{
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
