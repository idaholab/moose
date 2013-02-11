#include "Moose.h"
#include "Factory.h"

// Example 17 Includes
#include "Convection.h"
#include "ExampleDirac.h"         // <- New include for our custom DiracKernel

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
    registerDiracKernel(ExampleDirac);  // <- registration
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
