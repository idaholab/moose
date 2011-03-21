#include "AuxVariablesBlock.h"

// Moose includes
#include "Moose.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

// libMesh includes
#include "libmesh.h"
#include "mesh.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "getpot.h"
#include "numeric_vector.h"

template<>
InputParameters validParams<AuxVariablesBlock>()
{
  return validParams<ParserBlock>();
}

AuxVariablesBlock::AuxVariablesBlock(const std::string & name, InputParameters params) :
    VariablesBlock(name, params)
{
#if 0
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
#endif
}

void
AuxVariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxVariablesBlock Object\n";
#endif

  if (!_executed)
  {
    visitChildren();
    _executed = true;
  }
}

