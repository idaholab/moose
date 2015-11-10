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

// MOOSE includes
#include "AddSplitAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

template<>
InputParameters validParams<AddSplitAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addParam<std::string>("type", "Split", "Classname of the split object");
  return params;
}


AddSplitAction::AddSplitAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
AddSplitAction::act()
{
  _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem.get();
  _problem->getNonlinearSystem().addSplit(_type, _name, _moose_object_pars);
}
