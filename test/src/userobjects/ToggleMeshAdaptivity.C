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

#include "ToggleMeshAdaptivity.h"

template <>
InputParameters
validParams<ToggleMeshAdaptivity>()
{
  MooseEnum state("on off");

  InputParameters params = validParams<GeneralUserObject>();
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
