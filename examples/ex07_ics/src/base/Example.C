#include "Moose.h"
#include "Factory.h"

// Example 7 Includes
#include "ExampleIC.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    // Register our custom Initial Condition with the Factory
    registerInitialCondition(ExampleIC);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
