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

#include "SetupTimeIntegratorAction.h"
#include "Transient.h"
#include "Factory.h"
#include "TimeStepper.h"

template <>
InputParameters
validParams<SetupTimeIntegratorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

SetupTimeIntegratorAction::SetupTimeIntegratorAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
SetupTimeIntegratorAction::act()
{
  _problem->addTimeIntegrator(_type, _name, _moose_object_pars);
}
