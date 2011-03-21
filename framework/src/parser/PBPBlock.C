/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PBPBlock.h"
#include "PhysicsBasedPreconditioner.h"
#include "Moose.h"
#include "Parser.h"
#include "MProblem.h"
#include "ImplicitSystem.h"
#include "PhysicsBasedPreconditioner.h"

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

PBPBlock::PBPBlock(const std::string & name, InputParameters params) :
    ParserBlock(name, params)
{}

void
PBPBlock::execute() 
{
#if 0
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
#endif

  if (_parser_handle._problem != NULL)
  {
    Moose::MProblem & subproblem = *_parser_handle._problem;
    Moose::ImplicitSystem & nl = subproblem.getNonlinearSystem();

    unsigned int n_vars = nl.sys().n_vars();

    // solve order
    std::vector<unsigned int> solve_order;
    for(unsigned int i=0;i<getParamValue<std::vector<std::string> >("solve_order").size();i++)
      solve_order.push_back(nl.sys().variable_number(getParamValue<std::vector<std::string> >("solve_order")[i]));

    // precond type
    std::vector<PreconditionerType> pre;
    for(unsigned int i=0;i<getParamValue<std::vector<std::string> >("preconditioner").size();i++)
      pre.push_back(Utility::string_to_enum<PreconditionerType>(getParamValue<std::vector<std::string> >("preconditioner")[i]));

    // off-diagonal entries
    std::vector<std::vector<unsigned int> > off_diag(n_vars);
    for(unsigned int i=0;i<getParamValue<std::vector<std::string> >("off_diag_row").size();i++)
    {
      unsigned int row = nl.sys().variable_number(getParamValue<std::vector<std::string> >("off_diag_row")[i]);
      unsigned int column = nl.sys().variable_number(getParamValue<std::vector<std::string> >("off_diag_column")[i]);

//      //The +2 is because the preconditioning system is always 2 more than the variable number
//      //This is because of the Nonlinear and Auxiliary systems
//      LinearImplicitSystem & u_system = _moose_system.getEquationSystems()->get_system<LinearImplicitSystem>(row+2);
//
//      //Add the matrix to hold the off diagonal piece
//      u_system.add_matrix(getParamValue<std::vector<std::string> >("off_diag_column")[i]);

      off_diag[row].push_back(column);
    }

    // build preconditioner
    Moose::PhysicsBasedPreconditioner *precond = new Moose::PhysicsBasedPreconditioner(nl);

    // Add all of the preconditioning systems
    for(unsigned int var = 0; var < n_vars; var++)
    {
      precond->addSystem(var, off_diag[var], pre[var]);
/*
      std::string var_name = sys.sys().variable_name(var);

      LinearImplicitSystem & precond_system = _moose_system.getEquationSystems()->add_system<LinearImplicitSystem>(var_name+"_system");
      precond_system.assemble_before_solve = false;
      precond_system.add_variable(var_name+"_prec",
                                  system.variable(var).type().order,
                                  system.variable(var).type().family);
*/
    }

    precond->setSolveOrder(solve_order);
//    precond->setPreconditionerType(pre);
//    precond->setOffDiagBlocks(off_diag);

    nl.setPreconditioner(precond);
  }

  visitChildren();
}  
