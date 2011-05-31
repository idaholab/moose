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

#include "SetupSingleMatrixAction.h"
#include "Moose.h"
#include "Parser.h"
#include "MProblem.h"
#include "NonlinearSystem.h"

#include "string_to_enum.h"

template<>
InputParameters validParams<SetupSingleMatrixAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::string>("preconditioner", "TODO: docstring");
  
  params.addParam<std::vector<std::string> >("off_diag_row", "TODO: docstring");
  params.addParam<std::vector<std::string> >("off_diag_column", "TODO: docstring");
  return params;
}

SetupSingleMatrixAction::SetupSingleMatrixAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupSingleMatrixAction::act()
{
  if (_parser_handle._problem != NULL)
  {
    MProblem & subproblem = *_parser_handle._problem;
    NonlinearSystem & nl = subproblem.getNonlinearSystem();
    unsigned int n_vars = nl.nVariables();

    CouplingMatrix * cm = new CouplingMatrix(n_vars);
    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; i++)
      (*cm)(i, i) = 1;
    // off-diagonal entries
    std::vector<std::vector<unsigned int> > off_diag(n_vars);
    for(unsigned int i = 0; i < getParam<std::vector<std::string> >("off_diag_row").size(); i++)
    {
      unsigned int row = nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_row")[i]).number();
      unsigned int column = nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_column")[i]).number();
      (*cm)(row, column) = 1;
    }

    nl.couplingMatrix(cm);
  }
}  
