#include "AuxVariablesBlock.h"

// Moose includes
#include "Moose.h"
#include "AuxFactory.h"

// libMesh includes
#include "libmesh.h"
#include "mesh.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "getpot.h"

template<>
InputParameters validParams<AuxVariablesBlock>()
{
  return validParams<ParserBlock>();
}

AuxVariablesBlock::AuxVariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :VariablesBlock(reg_id, real_id, parent, parser_handle, params)
{
  // Register execution prereqs
  addPrereq("Mesh");
  addPrereq("Variables");
}

void
AuxVariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxVariablesBlock Object\n";
#endif

  TransientNonlinearImplicitSystem &system =
    Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

  TransientExplicitSystem& aux_system =
    Moose::equation_system->add_system<TransientExplicitSystem> ("AuxiliarySystem");
  
  visitChildren();

  aux_system.attach_init_function(Moose::init_cond);

  // Add a temporary vector for general use
  system.add_vector("temp", false);

  ParserBlock * pb = locateBlock("BCs/Periodic");

  if(pb)
    pb->execute();

  Moose::equation_system->init();
  Moose::equation_system->print_info();

  // Copy out nodal values is required (Variables Block)
  if (VariablesBlock * vars = dynamic_cast<VariablesBlock *>(locateBlock("Variables")))
    vars->copyNodalValues("NonlinearSystem");

  // Aux Variables
  this->copyNodalValues("AuxiliarySystem");
}
