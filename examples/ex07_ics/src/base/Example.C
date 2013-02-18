#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 7 Includes
#include "ExampleIC.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
    // Register our custom Initial Condition with the Factory
    registerInitialCondition(ExampleIC);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
