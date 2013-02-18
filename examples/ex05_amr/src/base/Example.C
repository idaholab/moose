#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 5 Registration
#include "Convection.h"
#include "ExampleCoefDiffusion.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
     registerKernel(Convection);
     registerKernel(ExampleCoefDiffusion);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
