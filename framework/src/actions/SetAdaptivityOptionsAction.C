//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetAdaptivityOptionsAction.h"
#include "FEProblem.h"

#include "libmesh/fe.h"

template <>
InputParameters
validParams<SetAdaptivityOptionsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<MarkerName>("marker",
                              "The name of the Marker to use to actually adapt the mesh.");
  params.addParam<unsigned int>(
      "steps", 0, "The number of adaptive steps to use when doing a Steady simulation.");
  params.addParam<unsigned int>(
      "initial_steps", 0, "The number of adaptive steps to do based on the initial condition.");
  params.addParam<MarkerName>(
      "initial_marker",
      "The name of the Marker to use to adapt the mesh during initial refinement.");
  params.addParam<unsigned int>(
      "max_h_level",
      0,
      "Maximum number of times a single element can be refined. If 0 then infinite.");
  params.addParam<Real>("start_time",
                        -std::numeric_limits<Real>::max(),
                        "The time that adaptivity will be active after.");
  params.addParam<Real>("stop_time",
                        std::numeric_limits<Real>::max(),
                        "The time after which adaptivity will no longer be active.");
  params.addParam<unsigned int>(
      "cycles_per_step",
      1,
      "The number of adaptive steps to use when on each timestep during a Transient simulation.");
  params.addParam<bool>(
      "recompute_markers_during_cycles", false, "Recompute markers during adaptivity cycles");
  return params;
}

SetAdaptivityOptionsAction::SetAdaptivityOptionsAction(InputParameters params) : Action(params) {}

void
SetAdaptivityOptionsAction::act()
{
  Adaptivity & adapt = _problem->adaptivity();

  if (isParamValid("marker"))
    adapt.setMarkerVariableName(getParam<MarkerName>("marker"));
  if (isParamValid("initial_marker"))
    adapt.setInitialMarkerVariableName(getParam<MarkerName>("initial_marker"));

  adapt.setCyclesPerStep(getParam<unsigned int>("cycles_per_step"));

  adapt.setMaxHLevel(getParam<unsigned int>("max_h_level"));

  adapt.init(getParam<unsigned int>("steps"), getParam<unsigned int>("initial_steps"));
  adapt.setUseNewSystem();

  adapt.setTimeActive(getParam<Real>("start_time"), getParam<Real>("stop_time"));

  adapt.setRecomputeMarkersFlag(getParam<bool>("recompute_markers_during_cycles"));
}
