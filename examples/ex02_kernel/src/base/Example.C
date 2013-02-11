#include "Moose.h"
#include "Factory.h"

// Example 2 Includes
#include "Convection.h"           // <- New include for our custom kernel

namespace Example
{
  void registerObjects(Factory & factory)
  {
    // Register any custom objects you have built on the MOOSE Framework
    registerKernel(Convection);  // <- registration
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
