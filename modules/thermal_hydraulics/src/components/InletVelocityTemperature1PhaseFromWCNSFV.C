//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletVelocityTemperature1PhaseFromWCNSFV.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", InletVelocityTemperature1PhaseFromWCNSFV);

InputParameters
InletVelocityTemperature1PhaseFromWCNSFV::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addRequiredParam<MooseFunctorName>("vel", "Inlet velocity [m/s]");
  params.addRequiredParam<MooseFunctorName>("T", "Inlet temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");

  params.addClassDescription(
      "Boundary condition with velocity and temperature from a WCNSFV simulation"
      "for 1-phase flow channels.");
  return params;
}

InletVelocityTemperature1PhaseFromWCNSFV::InletVelocityTemperature1PhaseFromWCNSFV(
    const InputParameters & params)
  : FlowBoundary1Phase(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletVelocityTemperature1PhaseFromWCNSFV::check() const
{
  FlowBoundary1Phase::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");
}

void
InletVelocityTemperature1PhaseFromWCNSFV::addMooseObjects()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "ADBoundaryFlux3EqnGhostFunctorVelocityTemperature";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MooseFunctorName>("vel") = getParam<MooseFunctorName>("vel");
    params.set<MooseFunctorName>("T") = getParam<MooseFunctorName>("T");
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    getTHMProblem().addUserObject(class_name, _boundary_uo_name, params);
    // connectObject(params, _boundary_uo_name, "vel");
    // connectObject(params, _boundary_uo_name, "T");
  }

  // BCs
  addWeakBC3Eqn();
}
