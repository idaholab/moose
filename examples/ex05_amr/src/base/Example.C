#include "Moose.h"
#include "Factory.h"

// Example 5 Registration
#include "Convection.h"
#include "ExampleCoefDiffusion.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
     registerKernel(Convection);
     registerKernel(ExampleCoefDiffusion);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
