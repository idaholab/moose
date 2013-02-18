#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 3 Includes
#include "Convection.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
