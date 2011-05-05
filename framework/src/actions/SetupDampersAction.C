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

#include "SetupDampersAction.h"
#include "Moose.h"
#include "Parser.h"
#include "MProblem.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<SetupDampersAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


SetupDampersAction::SetupDampersAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetupDampersAction::act()
{
  // if we have add_damper action, we will setup dampers ;-)
  ActionIterator it_beg = Moose::action_warehouse.actionBlocksWithActionBegin("add_damper");
  ActionIterator it_end = Moose::action_warehouse.actionBlocksWithActionEnd("add_damper");
  if (it_beg != it_end)
    _problem->setupDampers();
}
