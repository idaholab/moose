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

#include "SetupDebugAction.h"

template<>
InputParameters validParams<SetupDebugAction>()
{
  InputParameters params = validParams<Action>();

  return params;
}

SetupDebugAction::SetupDebugAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters)
{
}

SetupDebugAction::~SetupDebugAction()
{
}

void
SetupDebugAction::act()
{
}


