#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 14 Includes
#include "ExampleFunction.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
    registerFunction(ExampleFunction);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
