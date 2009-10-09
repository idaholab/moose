#include "VariablesBlock.h"

// Moose includes
#include "Moose.h"
#include "ComputeResidual.h"
#include "ComputeJacobian.h"

#include "AuxFactory.h"

// libMesh includes
#include "libmesh.h"
#include "mesh.h"
#include "equation_systems.h"
#include "nonlinear_solver.h"
#include "nonlinear_implicit_system.h"
#include "getpot.h"

VariablesBlock::VariablesBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{}

void
VariablesBlock::execute() 
{
#ifdef DEBUG
  std::cerr << "Inside the VariablesBlock Object\n";
#endif

  EquationSystems *equation_system = new EquationSystems(*Moose::mesh);
  Moose::equation_system = equation_system;
  TransientNonlinearImplicitSystem &system =
    equation_system->add_system<TransientNonlinearImplicitSystem>("NonlinearSystem");
  
  // Add variable blocks from the children nodes
  visitChildren();

  system.nonlinear_solver->residual = Moose::compute_residual;
  system.nonlinear_solver->jacobian = Moose::compute_jacobian;  
}
