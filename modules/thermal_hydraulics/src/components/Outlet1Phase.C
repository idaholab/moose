//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Outlet1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", Outlet1Phase);

InputParameters
Outlet1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addRequiredParam<Real>("p", "Prescribed pressure [Pa]");
  params.declareControllable("p");
  params.addClassDescription(
      "Boundary condition with prescribed pressure for 1-phase flow channels.");
  return params;
}

Outlet1Phase::Outlet1Phase(const InputParameters & params) : FlowBoundary1Phase(params) {}

void
Outlet1Phase::check() const
{
  FlowBoundary1Phase::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");
}

void
Outlet1Phase::addMooseObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "ADBoundaryFlux3EqnGhostPressure";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("p") = getParam<Real>("p");
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);
    connectObject(params, _boundary_uo_name, "p");
  }

  // BCs
  addWeakBC3Eqn();
}
