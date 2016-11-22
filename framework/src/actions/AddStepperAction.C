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

#include "AddStepperAction.h"
#include "FEProblem.h"
#include "Stepper.h"

template<>
InputParameters validParams<AddStepperAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddStepperAction::AddStepperAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
AddStepperAction::act()
{
  _problem->addStepper(_type, _name, _moose_object_pars);
}
