//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletDensityVelocity1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", InletDensityVelocity1Phase);

InputParameters
InletDensityVelocity1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addRequiredParam<Real>("rho", "Prescribed density [kg/m^3]");
  params.addRequiredParam<Real>("vel", "Prescribed velocity [m/s]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("rho vel");
  params.addClassDescription(
      "Boundary condition with prescribed density and velocity for 1-phase flow channels.");
  return params;
}

InletDensityVelocity1Phase::InletDensityVelocity1Phase(const InputParameters & params)
  : FlowBoundary1Phase(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletDensityVelocity1Phase::check() const
{
  FlowBoundary1Phase::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");
}

void
InletDensityVelocity1Phase::addMooseObjects()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "ADBoundaryFlux3EqnGhostDensityVelocity";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("rho") = getParam<Real>("rho");
    params.set<Real>("vel") = getParam<Real>("vel");
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);

    connectObject(params, _boundary_uo_name, "rho");
    connectObject(params, _boundary_uo_name, "vel");
  }

  // BCs
  addWeakBC3Eqn();
}
