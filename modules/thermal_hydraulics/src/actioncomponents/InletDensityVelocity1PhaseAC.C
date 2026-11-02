//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletDensityVelocity1PhaseAC.h"
#include "THMUtils.h"
#include "FlowChannel1PhasePhysics.h"
#include "MooseUtils.h"

// Registered under the classic Component's name ('InletDensityVelocity1Phase') rather than this
// class's own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletDensityVelocity1PhaseAC,
                           "InletDensityVelocity1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletDensityVelocity1PhaseAC,
                           "InletDensityVelocity1Phase",
                           "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp",
                               InletDensityVelocity1PhaseAC,
                               "InletDensityVelocity1Phase");

InputParameters
InletDensityVelocity1PhaseAC::validParams()
{
  InputParameters params = FlowBoundary1PhaseAC::validParams();
  params.addRequiredParam<Real>("rho", "Prescribed density [kg/m^3]");
  params.addRequiredParam<Real>("vel", "Prescribed velocity [m/s]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("rho vel");
  params.addClassDescription(
      "Boundary condition with prescribed density and velocity for a FlowChannel1PhaseAC.");
  return params;
}

InletDensityVelocity1PhaseAC::InletDensityVelocity1PhaseAC(const InputParameters & params)
  : ActionComponent(params),
    FlowBoundary1PhaseAC(params),
    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _reversible(getParam<bool>("reversible"))
{
}

void
InletDensityVelocity1PhaseAC::addUserObjects()
{
  const auto & physics = connectedPhysics();

  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const std::string class_name = "ADBoundaryFlux3EqnGhostDensityVelocity";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("rho") = _rho;
  params.set<Real>("vel") = _vel;
  params.set<Real>("normal") = _normal;
  params.set<bool>("reversible") = _reversible;
  params.set<UserObjectName>("fluid_properties") = physics.fluidPropertiesName();
  params.set<UserObjectName>("numerical_flux") = physics.numericalFluxUserObjectName();
  params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
  getProblem().addUserObject(class_name, _boundary_uo_name, params);
  THM::connectComponentControllableParam(*this, "rho", class_name, _boundary_uo_name);
  THM::connectComponentControllableParam(*this, "vel", class_name, _boundary_uo_name);
}
