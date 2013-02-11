#include "Moose.h"
#include "Factory.h"

// Example 18 Includes
#include "ScalarDirichletBC.h"
#include "ImplicitODEx.h"
#include "ImplicitODEy.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerBoundaryCondition(ScalarDirichletBC);
    registerScalarKernel(ImplicitODEx);
    registerScalarKernel(ImplicitODEy);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
