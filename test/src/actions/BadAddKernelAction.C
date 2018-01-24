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

#include "BadAddKernelAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<BadAddKernelAction>()
{
  return validParams<MooseObjectAction>();
}

BadAddKernelAction::BadAddKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
BadAddKernelAction::act()
{
  // Wrong method being called for adding *Kernel* object
  _problem->addScalarKernel(_type, _name, _moose_object_pars);
}
