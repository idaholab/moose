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
#include "InstantiatePostprocessorsAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<InstantiatePostprocessorsAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

InstantiatePostprocessorsAction::InstantiatePostprocessorsAction(InputParameters params) :
  Action(params)
{
}

InstantiatePostprocessorsAction::~InstantiatePostprocessorsAction()
{
}

void
InstantiatePostprocessorsAction::act()
{
  _problem->instantiatePostprocessors();
}
