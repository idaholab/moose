//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FreeBoundary1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", FreeBoundary1Phase);

InputParameters
FreeBoundary1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  return params;
}

FreeBoundary1Phase::FreeBoundary1Phase(const InputParameters & parameters)
  : FlowBoundary1Phase(parameters)
{
}

void
FreeBoundary1Phase::addMooseObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "ADBoundaryFlux3EqnFreeOutflow";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);
  }

  // BCs
  addWeakBC3Eqn();
}
