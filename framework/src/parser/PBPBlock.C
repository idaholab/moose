#include "PBPBlock.h"
#include "PhysicsBasedPreconditioner.h"
#include "Moose.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"

#include "linear_implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "nonlinear_solver.h"
#include "string_to_enum.h"

template<>
InputParameters validParams<PBPBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addRequiredParam<std::vector<std::string> >("solve_order", "TODO: docstring");
  params.addRequiredParam<std::vector<std::string> >("preconditioner", "TODO: docstring");
  
  params.addParam<std::vector<std::string> >("off_diag_row", "TODO: docstring");
  params.addParam<std::vector<std::string> >("off_diag_column", "TODO: docstring");
  return params;
}

PBPBlock::PBPBlock(std::string name, MooseSystem & moose_system, InputParameters params)
  :ParserBlock(name, moose_system, params)
{}

void
PBPBlock::execute() 
{
  TransientNonlinearImplicitSystem & system = *_moose_system.getNonlinearSystem();

  // We don't want to be computing the big Jacobian!
  system.nonlinear_solver->jacobian = NULL;
  
  // Add all of the preconditioning systems
  for(unsigned int var = 0; var < system.n_vars(); var++)
  {
    std::string var_name = system.variable_name(var);    
    LinearImplicitSystem & precond_system = _moose_system.getEquationSystems()->add_system<LinearImplicitSystem>(var_name+"_system");
    precond_system.assemble_before_solve = false;
    precond_system.add_variable(var_name+"_prec",
                                system.variable(var).type().order,
                                system.variable(var).type().family);
  }

  PhysicsBasedPreconditioner *precond = new PhysicsBasedPreconditioner(_moose_system);

  _moose_system.setPreconditioner(precond);
  
  std::vector<unsigned int> solve_order;
      
  for(unsigned int i=0;i<getParamValue<std::vector<std::string> >("solve_order").size();i++)
    solve_order.push_back(system.variable_number(getParamValue<std::vector<std::string> >("solve_order")[i]));

  std::vector<PreconditionerType> pre;

  for(unsigned int i=0;i<getParamValue<std::vector<std::string> >("preconditioner").size();i++)
    pre.push_back(Utility::string_to_enum<PreconditionerType>(getParamValue<std::vector<std::string> >("preconditioner")[i]));


  std::vector<std::vector<unsigned int> > off_diag(system.n_vars());

  for(unsigned int i=0;i<getParamValue<std::vector<std::string> >("off_diag_row").size();i++)
  {
    unsigned int row = system.variable_number(getParamValue<std::vector<std::string> >("off_diag_row")[i]);
    unsigned int column = system.variable_number(getParamValue<std::vector<std::string> >("off_diag_column")[i]);

    //The +2 is because the preconditioning system is always 2 more than the variable number
    //This is because of the Nonlinear and Auxiliary systems
    LinearImplicitSystem & u_system = _moose_system.getEquationSystems()->get_system<LinearImplicitSystem>(row+2);

    //Add the matrix to hold the off diagonal piece
    u_system.add_matrix(getParamValue<std::vector<std::string> >("off_diag_column")[i]);
        
    off_diag[row].push_back(column);
  }  
      
  precond->setSolveOrder(solve_order);
  precond->setPreconditionerType(pre);
  precond->setOffDiagBlocks(off_diag);
  
  system.nonlinear_solver->attach_preconditioner(precond);    

  visitChildren();
}  
