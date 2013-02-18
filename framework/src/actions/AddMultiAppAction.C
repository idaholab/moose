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

#include "AddMultiAppAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddMultiAppAction>()
{
  return validParams<MooseObjectAction>();
}

AddMultiAppAction::AddMultiAppAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddMultiAppAction::act()
{
  _problem->addMultiApp(_type, getShortName(), _moose_object_pars);
}
