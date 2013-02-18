#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example Includes
#include "BlockAverageDiffusionMaterial.h"
#include "BlockAverageValue.h"
#include "ExampleDiffusion.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

  void registerObjects(Factory & factory)
  {
    registerMaterial(BlockAverageDiffusionMaterial);
    registerKernel(ExampleDiffusion);

    // This is how to register a UserObject
    registerUserObject(BlockAverageValue);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
