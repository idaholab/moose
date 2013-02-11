#include "Moose.h"
#include "Factory.h"

// Example 3 Includes
#include "Convection.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
