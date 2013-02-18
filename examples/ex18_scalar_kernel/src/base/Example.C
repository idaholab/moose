#include "Example.h"
#include "ExampleApp.h"
#include "Moose.h"
#include "Factory.h"
#include "AppFactory.h"

// Example 18 Includes
#include "ScalarDirichletBC.h"
#include "ImplicitODEx.h"
#include "ImplicitODEy.h"

namespace Example
{
  void registerApps()
  {
    registerApp(ExampleApp);
  }

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
