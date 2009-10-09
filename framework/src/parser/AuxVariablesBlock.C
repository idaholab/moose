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

AuxVariablesBlock::AuxVariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{}

void
AuxVariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the AuxVariablesBlock Object\n";
#endif

  TransientExplicitSystem& aux_system =
    Moose::equation_system->add_system<TransientExplicitSystem> ("AuxiliarySystem");

  // TODO: Implement GenericAuxVariableBlock
  visitChildren();
  
  Moose::equation_system->init();
  Moose::equation_system->print_info();
}
