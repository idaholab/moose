#include "AuxVariablesBlock.h"

// Moose includes
#include "Moose.h"
#include "Factory.h"

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
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
//  addPrereq("Preconditioning");
}

void
AuxVariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxVariablesBlock Object\n";
#endif

//  TransientNonlinearImplicitSystem &system = *_moose_system.getNonlinearSystem();
  
  visitChildren();

  // Add a temporary vector for general use
//  system.add_vector("temp", false);

  ParserBlock * pb = locateBlock("BCs/Periodic");

  if (pb)
    pb->execute();

//  _moose_system.init();
//  _moose_system.getEquationSystems()->print_info();

//  // Copy out nodal values is required (Variables Block)
//  if (VariablesBlock * vars = dynamic_cast<VariablesBlock *>(locateBlock("Variables")))
//    vars->copyNodalValues("NonlinearSystem");

  // Aux Variables
//  this->copyNodalValues("AuxiliarySystem");
}

