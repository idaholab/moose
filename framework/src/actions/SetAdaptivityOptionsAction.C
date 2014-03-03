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

#include "SetAdaptivityOptionsAction.h"
#include "FEProblem.h"

// libmesh includes
#include "libmesh/fe.h"

template<>
InputParameters validParams<SetAdaptivityOptionsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<MarkerName>("marker", "The name of the Marker to use to actually adapt the mesh.");
  params.addParam<unsigned int>("steps", 0, "The number of adaptive steps to use when doing a Steady simulation.");
  params.addParam<unsigned int>("initial_steps", 0, "The number of adaptive steps to do based on the initial condition.");
  params.addParam<MarkerName>("initial_marker", "The name of the Marker to use to adapt the mesh during initial refinement.");
  params.addParam<unsigned int> ("max_h_level", 0, "Maximum number of times a single element can be refined. If 0 then infinite.");
  params.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that adaptivity will be active after.");
  params.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which adaptivity will no longer be active.");
  return params;
}

SetAdaptivityOptionsAction::SetAdaptivityOptionsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetAdaptivityOptionsAction::act()
{
  Adaptivity & adapt = _problem->adaptivity();

  if (isParamValid("marker"))
    adapt.setMarkerVariableName(getParam<MarkerName>("marker"));
  if (isParamValid("initial_marker"))
    adapt.setInitialMarkerVariableName(getParam<MarkerName>("initial_marker"));

  adapt.setMaxHLevel(getParam<unsigned int>("max_h_level"));

  adapt.init(getParam<unsigned int>("steps"), getParam<unsigned int>("initial_steps"));
  adapt.setUseNewSystem();

  adapt.setTimeActive(getParam<Real>("start_time"), getParam<Real>("stop_time"));
}
