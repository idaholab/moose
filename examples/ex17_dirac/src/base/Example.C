#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 17 Includes
#include "Convection.h"
#include "ExampleDirac.h"         // <- New include for our custom DiracKernel

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
    registerDiracKernel(ExampleDirac);  // <- registration
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
