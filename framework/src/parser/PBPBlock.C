#include "PBPBlock.h"
#include "PhysicsBasedPreconditioner.h"
#include "ComputeJacobian.h"

#include "linear_implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "nonlinear_solver.h"
#include "string_to_enum.h"

PBPBlock::PBPBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file)
{
  addParam<std::vector<std::string> >("solve_order", "TODO: docstring", true);
  addParam<std::vector<std::string> >("preconditioner", "TODO: docstring", true);
  addParam<std::vector<std::string> >("off_diag_row", "TODO: docstring", false);
  addParam<std::vector<std::string> >("off_diag_column", "TODO: docstring", false);
}

void
PBPBlock::execute() 
{
  TransientNonlinearImplicitSystem & system = Moose::equation_system->get_system<TransientNonlinearImplicitSystem>("NonlinearSystem");

  // We don't want to be computing the big Jacobian!
  system.nonlinear_solver->jacobian = NULL;
  
  // Add all of the preconditioning systems
  for(unsigned int var = 0; var < system.n_vars(); var++)
  {
    std::string var_name = system.variable_name(var);    
    LinearImplicitSystem & precond_system = Moose::equation_system->add_system<LinearImplicitSystem>(var_name+"_system");
    precond_system.assemble_before_solve = false;
    precond_system.add_variable(var_name+"_prec",
                                system.variable(var).type().order,
                                system.variable(var).type().family);
  }

  PhysicsBasedPreconditioner *precond = new PhysicsBasedPreconditioner();
  
  std::vector<unsigned int> solve_order;
      
  for(int i=0;i<getParamValue<std::vector<std::string> >("solve_order").size();i++)
    solve_order.push_back(system.variable_number(getParamValue<std::vector<std::string> >("solve_order")[i]));

  std::vector<PreconditionerType> pre;

  for(int i=0;i<getParamValue<std::vector<std::string> >("preconditioner").size();i++)
    pre.push_back(Utility::string_to_enum<PreconditionerType>(getParamValue<std::vector<std::string> >("preconditioner")[i]));


  std::vector<std::vector<unsigned int> > off_diag(system.n_vars());

  for(int i=0;i<getParamValue<std::vector<std::string> >("off_diag_row").size();i++)
  {
    unsigned int row = system.variable_number(getParamValue<std::vector<std::string> >("off_diag_row")[i]);
    unsigned int column = system.variable_number(getParamValue<std::vector<std::string> >("off_diag_column")[i]);

    //The +1 is because the preconditioning system is always 1 more than the variable number
    LinearImplicitSystem & u_system = Moose::equation_system->get_system<LinearImplicitSystem>(row+1);

    //Add the matrix to hold the off diagonal piece
    u_system.add_matrix(getParamValue<std::vector<std::string> >("off_diag_column")[i]);
        
    off_diag[row].push_back(column);
  }  
      
  precond->setEq(*Moose::equation_system);
  precond->setComputeJacobianBlock(Moose::compute_jacobian_block);
  precond->setSolveOrder(solve_order);
  precond->setPreconditionerType(pre);
  precond->setOffDiagBlocks(off_diag);
  
  system.nonlinear_solver->attach_preconditioner(precond);    

  visitChildren();
}  
