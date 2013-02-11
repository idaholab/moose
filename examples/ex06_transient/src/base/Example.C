#include "Moose.h"
#include "Factory.h"

// Example 6 Includes
#include "ExampleDiffusion.h"
#include "Convection.h"
#include "ExampleTimeDerivative.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
    registerKernel(ExampleDiffusion);
    registerKernel(ExampleTimeDerivative);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
