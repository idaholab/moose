//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletFunction1Phase.h"

registerMooseObject("ThermalHydraulicsTestApp", InletFunction1Phase);

InputParameters
InletFunction1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();

  params.addClassDescription("1-phase inlet with all variables prescribed by functions.");

  params.addRequiredParam<FunctionName>("rho", "Prescribed density [kg/m^3]");
  params.addRequiredParam<FunctionName>("vel", "Prescribed velocity [m/s]");
  params.addRequiredParam<FunctionName>("p", "Prescribed pressure [Pa]");

  return params;
}

InletFunction1Phase::InletFunction1Phase(const InputParameters & params)
  : FlowBoundary1Phase(params)
{
}

void
InletFunction1Phase::addMooseObjects()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "BoundaryFlux3EqnFunction";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<FunctionName>("rho") = getParam<FunctionName>("rho");
    params.set<FunctionName>("vel") = getParam<FunctionName>("vel");
    params.set<FunctionName>("p") = getParam<FunctionName>("p");
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);
  }

  // BCs
  addWeakBC3Eqn();
}
