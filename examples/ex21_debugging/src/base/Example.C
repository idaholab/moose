#include "Moose.h"
#include "Factory.h"

// Example 21 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
    registerKernel(ExampleDiffusion);
    registerMaterial(ExampleMaterial);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
