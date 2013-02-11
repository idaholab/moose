#include "Moose.h"
#include "Factory.h"

// Example 16 Includes
#include "TransientHalf.h"
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleImplicitEuler.h"
#include "ExampleMaterial.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
     // Register our new executioner
    registerExecutioner(TransientHalf);
    registerKernel(ExampleDiffusion);
    registerKernel(Convection);
    registerKernel(ExampleImplicitEuler);
    registerMaterial(ExampleMaterial);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
