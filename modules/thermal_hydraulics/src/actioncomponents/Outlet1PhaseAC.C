//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Outlet1PhaseAC.h"
#include "FlowChannel1PhasePhysics.h"
#include "MooseUtils.h"

// Registered under the classic Component's name ('Outlet1Phase') rather than this class's own
// name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           Outlet1PhaseAC,
                           "Outlet1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp", Outlet1PhaseAC, "Outlet1Phase", "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp", Outlet1PhaseAC, "Outlet1Phase");

InputParameters
Outlet1PhaseAC::validParams()
{
  InputParameters params = FlowBoundary1PhaseAC::validParams();
  params.addRequiredParam<Real>("p", "Prescribed pressure [Pa]");
  params.addClassDescription(
      "Boundary condition with prescribed pressure for a FlowChannel1PhaseAC.");
  return params;
}

Outlet1PhaseAC::Outlet1PhaseAC(const InputParameters & params)
  : ActionComponent(params), FlowBoundary1PhaseAC(params), _p(getParam<Real>("p"))
{
}

void
Outlet1PhaseAC::addUserObjects()
{
  const auto & physics = connectedPhysics();

  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const std::string class_name = "ADBoundaryFlux3EqnGhostPressure";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("p") = _p;
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("fluid_properties") = physics.fluidPropertiesName();
  params.set<UserObjectName>("numerical_flux") = physics.numericalFluxUserObjectName();
  params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
  getProblem().addUserObject(class_name, _boundary_uo_name, params);
}
