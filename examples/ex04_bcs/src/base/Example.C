#include "Moose.h"
#include "Factory.h"

// Example 4 Includes
#include "Convection.h"
#include "GaussContForcing.h"
#include "CoupledDirichletBC.h"
#include "CoupledNeumannBC.h"

namespace Example
{
  void registerObjects(Factory & factory)
  {
    registerKernel(Convection);
    registerKernel(GaussContForcing);                 // Extra forcing term
    registerBoundaryCondition(CoupledDirichletBC);    // Register our Boundary Conditions
    registerBoundaryCondition(CoupledNeumannBC);
  }

  void associateSyntax(Syntax & syntax, ActionFactory & action_factory)
  {
  }
}
