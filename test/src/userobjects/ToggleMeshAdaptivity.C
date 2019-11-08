//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ToggleMeshAdaptivity.h"

registerMooseObject("MooseTestApp", ToggleMeshAdaptivity);

InputParameters
ToggleMeshAdaptivity::validParams()
{
  MooseEnum state("on off");

  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<MooseEnum>(
      "mesh_adaptivity", state, "Control mesh adaptivity, choices are 'on' or 'off'.");
  params.addParam<int>("apply_after_timestep",
                       0,
                       "Number of time steps to wait before applying the adaptivity state.");
  return params;
}

ToggleMeshAdaptivity::ToggleMeshAdaptivity(const InputParameters & params)
  : GeneralUserObject(params),
    _state(getParam<MooseEnum>("mesh_adaptivity")),
    _steps_to_wait(getParam<int>("apply_after_timestep"))
{
}

void
ToggleMeshAdaptivity::checkState()
{
  if (_state == "on")
    _fe_problem.adaptivity().setAdaptivityOn(true);
  else
    _fe_problem.adaptivity().setAdaptivityOn(false);
}

void
ToggleMeshAdaptivity::initialSetup()
{
  // doing this here because mesh adaptivity appears to be re-initialized on recover
  // even though it was shut off previously in execute()
  if (_app.isRecovering())
    checkState();
}

void
ToggleMeshAdaptivity::execute()
{
  if (_t_step > _steps_to_wait)
    checkState();
}
