//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletVelocityTemperature1PhaseAC.h"
#include "THMUtils.h"
#include "FlowChannel1PhasePhysics.h"
#include "MooseUtils.h"

registerMooseActionAliased("ThermalHydraulicsApp",
                           InletVelocityTemperature1PhaseAC,
                           "InletVelocityTemperature1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletVelocityTemperature1PhaseAC,
                           "InletVelocityTemperature1Phase",
                           "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp",
                               InletVelocityTemperature1PhaseAC,
                               "InletVelocityTemperature1Phase");

InputParameters
InletVelocityTemperature1PhaseAC::validParams()
{
  InputParameters params = FlowBoundary1PhaseAC::validParams();
  params.addRequiredParam<Real>("vel", "Prescribed velocity [m/s]");
  params.addRequiredParam<Real>("T", "Prescribed temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("vel T");
  params.addClassDescription(
      "Boundary condition with prescribed velocity and temperature for a FlowChannel1PhaseAC.");
  return params;
}

InletVelocityTemperature1PhaseAC::InletVelocityTemperature1PhaseAC(const InputParameters & params)
  : ActionComponent(params),
    FlowBoundary1PhaseAC(params),
    _vel(getParam<Real>("vel")),
    _T(getParam<Real>("T")),
    _reversible(getParam<bool>("reversible"))
{
}

void
InletVelocityTemperature1PhaseAC::addUserObjects()
{
  const auto & physics = connectedPhysics();

  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const std::string class_name = "ADBoundaryFlux3EqnGhostVelocityTemperature";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("vel") = _vel;
  params.set<Real>("T") = _T;
  params.set<Real>("normal") = _normal;
  params.set<bool>("reversible") = _reversible;
  params.set<UserObjectName>("fluid_properties") = physics.fluidPropertiesName();
  params.set<UserObjectName>("numerical_flux") = physics.numericalFluxUserObjectName();
  params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
  getProblem().addUserObject(class_name, _boundary_uo_name, params);
  THM::connectComponentControllableParam(*this, "vel", class_name, _boundary_uo_name);
  THM::connectComponentControllableParam(*this, "T", class_name, _boundary_uo_name);
}
