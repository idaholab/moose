//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalHydraulicsApp.h"
#include "THMSyntax.h"
#include "HeatTransferApp.h"
#include "FluidPropertiesApp.h"
#include "NavierStokesApp.h"
#include "RayTracingApp.h"
#include "RdgApp.h"
#include "SolidPropertiesApp.h"
#include "MiscApp.h"

#include "AppFactory.h"
#include "Simulation.h"

#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

std::map<THM::FlowModelID, std::string> ThermalHydraulicsApp::_flow_model_map;

namespace THM
{

FlowModelID
registerFlowModelID()
{
  static FlowModelID flow_model_id = 0;
  flow_model_id++;
  return flow_model_id;
}

FlowModelID FM_INVALID = registerFlowModelID();
FlowModelID FM_SINGLE_PHASE = registerFlowModelID();
FlowModelID FM_TWO_PHASE = registerFlowModelID();
FlowModelID FM_TWO_PHASE_NCG = registerFlowModelID();

} // namespace THM

InputParameters
ThermalHydraulicsApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_output_syntax") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

registerKnownLabel("ThermalHydraulicsApp");

ThermalHydraulicsApp::ThermalHydraulicsApp(InputParameters parameters) : MooseApp(parameters)
{
  ThermalHydraulicsApp::registerAll(_factory, _action_factory, _syntax);
}

ThermalHydraulicsApp::~ThermalHydraulicsApp() {}

void
ThermalHydraulicsApp::registerObjects(Factory & /*factory*/)
{
  mooseError("registerObjects() is deprecated and not supported in THM");
}

void
ThermalHydraulicsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  mooseError("associateSyntax() is deprecated and not supported in THM");
}

void
ThermalHydraulicsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseError("registerExecFlags() is deprecated and not supported in THM");
}

void
ThermalHydraulicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ThermalHydraulicsApp"});
  Registry::registerActionsTo(af, {"ThermalHydraulicsApp"});

  HeatTransferApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  NavierStokesApp::registerAll(f, af, s);
  RayTracingApp::registerAll(f, af, s);
  RdgApp::registerAll(f, af, s);
  SolidPropertiesApp::registerAll(f, af, s);
  MiscApp::registerAll(f, af, s);

  THM::associateSyntax(s);
  THM::registerActions(s);

  // flow models
  registerFlowModel(THM::FM_SINGLE_PHASE, FlowModelSinglePhase);

  // Component variable ordering:
  // Note that this particular order ({rhoA, rhoEA, rhouA}) corresponds to the
  // the alphabetic ordering, which was the ordering used before this ordering
  // feature was implemented. We preserve this order for ease of transition,
  // but an order such as {rhoA, rhouA, rhoEA} may work as well.
  Simulation::setComponentVariableOrder(FlowModelSinglePhase::RHOA, 0);
  Simulation::setComponentVariableOrder(FlowModelSinglePhase::RHOEA, 1);
  Simulation::setComponentVariableOrder(FlowModelSinglePhase::RHOUA, 2);
  Simulation::setComponentVariableOrder("rhoV", 3);
  Simulation::setComponentVariableOrder("rhouV", 4);
  Simulation::setComponentVariableOrder("rhovV", 5);
  Simulation::setComponentVariableOrder("rhowV", 6);
  Simulation::setComponentVariableOrder("rhoEV", 7);
}

const std::string &
ThermalHydraulicsApp::getFlowModelClassName(const THM::FlowModelID & flow_model_id)
{
  const auto it = _flow_model_map.find(flow_model_id);
  if (it == _flow_model_map.end())
    mooseError("Flow model with ID '" + Moose::stringify(flow_model_id) +
               "' is not associated with any flow models class. Register your flow model class to "
               "a flow model ID by calling registerFlowModel().");
  return it->second;
}

void
ThermalHydraulicsApp::registerApps()
{
  registerApp(ThermalHydraulicsApp);

  HeatTransferApp::registerApps();
  FluidPropertiesApp::registerApps();
  NavierStokesApp::registerApps();
  RayTracingApp::registerApps();
  RdgApp::registerApps();
  SolidPropertiesApp::registerApps();
  MiscApp::registerApps();
}

//
// Dynamic Library Entry Points - DO NOT MODIFY
//
extern "C" void
ThermalHydraulicsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ThermalHydraulicsApp::registerAll(f, af, s);
}

extern "C" void
ThermalHydraulicsApp__registerApps()
{
  ThermalHydraulicsApp::registerApps();
}
