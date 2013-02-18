#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 10 Includes
#include "ExampleAux.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
     // Register our Example AuxKernel with the AuxFactory
    registerAux(ExampleAux);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
