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

#include "SetupOutputNamesAction.h"
#include "MooseApp.h"
#include "ActionWarehouse.h"
#include "OutputWarehouse.h"

template<>
InputParameters validParams<SetupOutputNamesAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

SetupOutputNamesAction::SetupOutputNamesAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

SetupOutputNamesAction::~SetupOutputNamesAction()
{
}

void
SetupOutputNamesAction::act()
{
  // Storage for a set of OutputNames
  std::set<std::string> avail;

  // Extract the names from the ActionWarehouse
  ActionWarehouse & awh = _app.actionWarehouse();
  const std::vector<Action *> & actions = awh.getActionsByName("add_output");
  for (std::vector<Action *>::const_iterator it = actions.begin(); it != actions.end(); ++it)
    avail.insert((*it)->getShortName());

  // Update the OutputWarehouse with the current names
  _app.getOutputWarehouse().setOutputNames(avail);
}
