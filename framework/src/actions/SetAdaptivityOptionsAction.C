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
#include "fe.h"

template<>
InputParameters validParams<SetAdaptivityOptionsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<MarkerName>("marker", "The name of the Marker to use to actually adapt the mesh.");
  params.addParam<unsigned int>("steps", 0, "The number of adaptive steps to use when doing a Steady simulation.");
  params.addParam<unsigned int>("initial_steps", 0, "The number of adaptive steps to do based on the initial condition.");
  params.addParam<MarkerName>("initial_marker", "The name of the Marker to use to adapt the mesh during initial refinement.");
  params.addParam<unsigned int> ("max_h_level", 0, "Maximum number of times a single element can be refined. If 0 then infinite.");
  return params;
}

SetAdaptivityOptionsAction::SetAdaptivityOptionsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
SetAdaptivityOptionsAction::act()
{
  if(isParamValid("marker"))
    _problem->adaptivity().setMarkerVariableName(getParam<MarkerName>("marker"));
  if(isParamValid("initial_marker"))
    _problem->adaptivity().setInitialMarkerVariableName(getParam<MarkerName>("initial_marker"));

  _problem->adaptivity().setMaxHLevel(getParam<unsigned int>("max_h_level"));

  _problem->adaptivity().init(getParam<unsigned int>("steps"), getParam<unsigned int>("initial_steps"));
  _problem->adaptivity().setUseNewSystem();
}
