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

#include "SetupTimeStepperAction.h"
#include "Transient.h"
#include "Factory.h"
#include "TimeStepper.h"

template<>
InputParameters validParams<SetupTimeStepperAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}

SetupTimeStepperAction::SetupTimeStepperAction(const std::string & name, InputParameters parameters) :
    MooseObjectAction(name, parameters)
{
}

SetupTimeStepperAction::~SetupTimeStepperAction()
{
}

void
SetupTimeStepperAction::act()
{
  if (_problem->isTransient())
  {
    Transient * transient = dynamic_cast<Transient *>(_executioner);
    if (transient == NULL)
      mooseError("You can setup time stepper only with executioners of transient type.");

    _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem;
    _moose_object_pars.set<Transient *>("_executioner") = transient;
    TimeStepper * ts = static_cast<TimeStepper *>(_factory.create(_type, "TimeStepper", _moose_object_pars));
    transient->setTimeStepper(ts);
  }
}
