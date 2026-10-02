//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InletMassFlowRateTemperature1PhaseAC.h"
#include "FlowChannel1PhasePhysics.h"
#include "MooseUtils.h"

// Registered under the classic Component's name ('InletMassFlowRateTemperature1Phase') rather
// than this class's own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletMassFlowRateTemperature1PhaseAC,
                           "InletMassFlowRateTemperature1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp",
                           InletMassFlowRateTemperature1PhaseAC,
                           "InletMassFlowRateTemperature1Phase",
                           "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp",
                               InletMassFlowRateTemperature1PhaseAC,
                               "InletMassFlowRateTemperature1Phase");

InputParameters
InletMassFlowRateTemperature1PhaseAC::validParams()
{
  InputParameters params = FlowBoundary1PhaseAC::validParams();
  params.addRequiredParam<Real>("m_dot", "Prescribed mass flow rate [kg/s]");
  params.addRequiredParam<Real>("T", "Prescribed temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.addClassDescription("Boundary condition with prescribed mass flow rate and temperature "
                             "for a FlowChannel1PhaseAC.");
  return params;
}

InletMassFlowRateTemperature1PhaseAC::InletMassFlowRateTemperature1PhaseAC(
    const InputParameters & params)
  : ActionComponent(params),
    FlowBoundary1PhaseAC(params),
    _m_dot(getParam<Real>("m_dot")),
    _T(getParam<Real>("T")),
    _reversible(getParam<bool>("reversible"))
{
}

void
InletMassFlowRateTemperature1PhaseAC::addUserObjects()
{
  const auto & physics = connectedPhysics();

  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const std::string class_name = "ADBoundaryFlux3EqnGhostMassFlowRateTemperature";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Real>("mass_flow_rate") = _m_dot;
  params.set<Real>("T") = _T;
  params.set<bool>("reversible") = _reversible;
  params.set<std::vector<FunctionName>>("passives") = {};
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("numerical_flux") = physics.numericalFluxUserObjectName();
  params.set<UserObjectName>("fluid_properties") = physics.fluidPropertiesName();
  params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
  getProblem().addUserObject(class_name, _boundary_uo_name, params);
}
