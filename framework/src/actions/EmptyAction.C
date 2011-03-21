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

#include "EmptyAction.h"

template<>
InputParameters validParams<EmptyAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


EmptyAction::EmptyAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
EmptyAction::act()
{
}
