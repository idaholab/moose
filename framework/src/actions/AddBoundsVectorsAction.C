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
#include "AddBoundsVectorsAction.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<AddBoundsVectorsAction>()
{
  return validParams<Action>();
}

AddBoundsVectorsAction::AddBoundsVectorsAction(InputParameters params) : Action(params) {}

void
AddBoundsVectorsAction::act()
{
  _problem->getNonlinearSystemBase().addVector("lower_bound", false, GHOSTED);
  _problem->getNonlinearSystemBase().addVector("upper_bound", false, GHOSTED);
}
