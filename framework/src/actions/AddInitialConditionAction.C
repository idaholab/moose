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

#include "AddInitialConditionAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddInitialConditionAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddInitialConditionAction::AddInitialConditionAction(InputParameters params)
  : MooseObjectAction(params)
{
}

void
AddInitialConditionAction::act()
{
  _problem->addInitialCondition(_type, _name, _moose_object_pars);
}
