#include "SetupPBPAction.h"
#include "PhysicsBasedPreconditioner.h"
#include "Moose.h"
#include "Parser.h"
#include "MProblem.h"
#include "NonlinearSystem.h"
#include "PhysicsBasedPreconditioner.h"

#include "string_to_enum.h"

template<>
InputParameters validParams<SetupPBPAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<std::string> >("solve_order", "TODO: docstring");
  params.addRequiredParam<std::vector<std::string> >("preconditioner", "TODO: docstring");
  
  params.addParam<std::vector<std::string> >("off_diag_row", "TODO: docstring");
  params.addParam<std::vector<std::string> >("off_diag_column", "TODO: docstring");
  return params;
}

SetupPBPAction::SetupPBPAction(const std::string & name, InputParameters params) :
    Action(name, params)
{}

void
SetupPBPAction::act() 
{
  if (_parser_handle._problem != NULL)
  {
    MProblem & subproblem = *_parser_handle._problem;
    NonlinearSystem & nl = subproblem.getNonlinearSystem();

    unsigned int n_vars = nl.sys().n_vars();

    // solve order
    std::vector<unsigned int> solve_order;
    for(unsigned int i=0;i<getParam<std::vector<std::string> >("solve_order").size();i++)
      solve_order.push_back(nl.sys().variable_number(getParam<std::vector<std::string> >("solve_order")[i]));

    // precond type
    std::vector<PreconditionerType> pre;
    for(unsigned int i=0;i<getParam<std::vector<std::string> >("preconditioner").size();i++)
      pre.push_back(Utility::string_to_enum<PreconditionerType>(getParam<std::vector<std::string> >("preconditioner")[i]));

    // off-diagonal entries
    std::vector<std::vector<unsigned int> > off_diag(n_vars);
    for(unsigned int i=0;i<getParam<std::vector<std::string> >("off_diag_row").size();i++)
    {
      unsigned int row = nl.sys().variable_number(getParam<std::vector<std::string> >("off_diag_row")[i]);
      unsigned int column = nl.sys().variable_number(getParam<std::vector<std::string> >("off_diag_column")[i]);
      off_diag[row].push_back(column);
    }

    // build preconditioner
    PhysicsBasedPreconditioner *precond = new PhysicsBasedPreconditioner(subproblem);

    // Add all of the preconditioning systems
    for(unsigned int var = 0; var < n_vars; var++)
      precond->addSystem(var, off_diag[var], pre[var]);

    precond->setSolveOrder(solve_order);
    nl.setPreconditioner(precond);
  }
}  
