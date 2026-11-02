//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletStagnationPressureTemperature1PhaseAC.h"
#include "THMUtils.h"
#include "FlowChannel1PhasePhysics.h"
#include "MooseUtils.h"

// Registered under the classic Component's name ('InletStagnationPressureTemperature1Phase')
// rather than this class's own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletStagnationPressureTemperature1PhaseAC,
                           "InletStagnationPressureTemperature1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletStagnationPressureTemperature1PhaseAC,
                           "InletStagnationPressureTemperature1Phase",
                           "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp",
                               InletStagnationPressureTemperature1PhaseAC,
                               "InletStagnationPressureTemperature1Phase");

InputParameters
InletStagnationPressureTemperature1PhaseAC::validParams()
{
  InputParameters params = FlowBoundary1PhaseAC::validParams();
  params.addRequiredParam<Real>("p0", "Prescribed stagnation pressure [Pa]");
  params.addRequiredParam<Real>("T0", "Prescribed stagnation temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("p0 T0");
  params.addClassDescription("Boundary condition with prescribed stagnation pressure and "
                             "temperature for a FlowChannel1PhaseAC.");
  return params;
}

InletStagnationPressureTemperature1PhaseAC::InletStagnationPressureTemperature1PhaseAC(
    const InputParameters & params)
  : ActionComponent(params),
    FlowBoundary1PhaseAC(params),
    _p0(getParam<Real>("p0")),
    _T0(getParam<Real>("T0")),
    _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationPressureTemperature1PhaseAC::addUserObjects()
{
  const auto & physics = connectedPhysics();

  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const std::string class_name = "ADBoundaryFlux3EqnGhostStagnationPressureTemperature";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("p0") = _p0;
  params.set<Real>("T0") = _T0;
  params.set<Real>("normal") = _normal;
  params.set<bool>("reversible") = _reversible;
  params.set<UserObjectName>("fluid_properties") = physics.fluidPropertiesName();
  params.set<UserObjectName>("numerical_flux") = physics.numericalFluxUserObjectName();
  params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
  getProblem().addUserObject(class_name, _boundary_uo_name, params);
  THM::connectComponentControllableParam(*this, "p0", class_name, _boundary_uo_name);
  THM::connectComponentControllableParam(*this, "T0", class_name, _boundary_uo_name);
}
