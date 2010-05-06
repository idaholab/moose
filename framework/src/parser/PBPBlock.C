#include "PBPBlock.h"
#include "PhysicsBasedPreconditioner.h"
#include "Moose.h"

#include "linear_implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "nonlinear_solver.h"
#include "string_to_enum.h"

// FIXME: remove me whne libmesh solver problem is fixed
namespace Moose {
void compute_residual (const NumericVector<Number>& soln, NumericVector<Number>& residual);
void compute_jacobian (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian);
void compute_jacobian_block (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar);
}

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

PBPBlock::PBPBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params)
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

  PhysicsBasedPreconditioner *precond = new PhysicsBasedPreconditioner();

  _moose_system.setPreconditioner(precond);
  
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
    LinearImplicitSystem & u_system = _moose_system.getEquationSystems()->get_system<LinearImplicitSystem>(row+1);

    //Add the matrix to hold the off diagonal piece
    u_system.add_matrix(getParamValue<std::vector<std::string> >("off_diag_column")[i]);
        
    off_diag[row].push_back(column);
  }  
      
  precond->setEq(*_moose_system.getEquationSystems());
  precond->setComputeJacobianBlock(Moose::compute_jacobian_block);
  precond->setSolveOrder(solve_order);
  precond->setPreconditionerType(pre);
  precond->setOffDiagBlocks(off_diag);
  
  system.nonlinear_solver->attach_preconditioner(precond);    

  visitChildren();
}  
