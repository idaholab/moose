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

#include "CheckIntegrityAction.h"
#include "ActionWarehouse.h"
#include "FEProblem.h"

template<>
InputParameters validParams<CheckIntegrityAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


CheckIntegrityAction::CheckIntegrityAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
CheckIntegrityAction::act()
{
  _awh.checkUnsatisfiedActions();
  if (_problem != NULL)
    _problem->checkProblemIntegrity();
}
