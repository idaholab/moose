#include "Moose.h"
#include "Factory.h"

// Example 8 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleMaterial.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
    // Our new Diffusion Kernel that accepts a material property
    registerKernel(ExampleDiffusion);
    // Register our new material class so we can use it.
    registerMaterial(ExampleMaterial);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
