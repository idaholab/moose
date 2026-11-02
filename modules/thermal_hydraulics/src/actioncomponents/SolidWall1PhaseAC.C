//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidWall1PhaseAC.h"
#include "FlowChannel1PhasePhysics.h"
#include "MooseUtils.h"

// Registered under the classic Component's name ('SolidWall1Phase') rather than this class's own
// name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           SolidWall1PhaseAC,
                           "SolidWall1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp", SolidWall1PhaseAC, "SolidWall1Phase", "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp", SolidWall1PhaseAC, "SolidWall1Phase");

InputParameters
SolidWall1PhaseAC::validParams()
{
  InputParameters params = FlowBoundary1PhaseAC::validParams();
  params.addClassDescription("Wall boundary condition for a FlowChannel1PhaseAC.");
  return params;
}

SolidWall1PhaseAC::SolidWall1PhaseAC(const InputParameters & params)
  : ActionComponent(params), FlowBoundary1PhaseAC(params)
{
}

void
SolidWall1PhaseAC::addUserObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const std::string class_name = "ADBoundaryFlux3EqnGhostWall";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("numerical_flux") = connectedPhysics().numericalFluxUserObjectName();
  params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
  getProblem().addUserObject(class_name, _boundary_uo_name, params);
}
