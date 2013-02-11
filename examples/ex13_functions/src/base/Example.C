#include "Moose.h"
#include "Factory.h"

// Example 13 Includes
#include "ExampleFunction.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerFunction(ExampleFunction);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
