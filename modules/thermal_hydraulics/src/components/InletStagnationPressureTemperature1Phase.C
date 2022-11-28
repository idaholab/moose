//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletStagnationPressureTemperature1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", InletStagnationPressureTemperature1Phase);

InputParameters
InletStagnationPressureTemperature1Phase::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addRequiredParam<Real>("p0", "Prescribed stagnation pressure [Pa]");
  params.addRequiredParam<Real>("T0", "Prescribed stagnation temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("p0 T0");
  params.addClassDescription("Boundary condition with prescribed stagnation pressure and "
                             "temperature for 1-phase flow channels.");
  return params;
}

InletStagnationPressureTemperature1Phase::InletStagnationPressureTemperature1Phase(
    const InputParameters & params)
  : FlowBoundary1Phase(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationPressureTemperature1Phase::check() const
{
  FlowBoundary1Phase::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");
}

void
InletStagnationPressureTemperature1Phase::addMooseObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  {
    const std::string class_name = "ADBoundaryFlux3EqnGhostStagnationPressureTemperature";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("p0") = getParam<Real>("p0");
    params.set<Real>("T0") = getParam<Real>("T0");
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);
    connectObject(params, _boundary_uo_name, "p0");
    connectObject(params, _boundary_uo_name, "T0");
  }

  // BCs
  addWeakBC3Eqn();
}
