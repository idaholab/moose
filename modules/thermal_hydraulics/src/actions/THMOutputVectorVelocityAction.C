//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMOutputVectorVelocityAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMOutputVectorVelocityAction,
                    "THM:output_vector_velocity");

InputParameters
THMOutputVectorVelocityAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>(
      "velocity_as_vector", true, "True for vector-valued velocity, false for scalar velocity.");
  return params;
}

THMOutputVectorVelocityAction::THMOutputVectorVelocityAction(const InputParameters & params)
  : Action(params)
{
}

void
THMOutputVectorVelocityAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setVectorValuedVelocity(getParam<bool>("velocity_as_vector"));
}
