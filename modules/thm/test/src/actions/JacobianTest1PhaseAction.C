#include "JacobianTest1PhaseAction.h"

registerMooseAction("THMTestApp", JacobianTest1PhaseAction, "meta_action");

template <>
InputParameters
validParams<JacobianTest1PhaseAction>()
{
  InputParameters params = emptyInputParameters();
  params += validParams<JacobianTestAction>();
  params += validParams<FlowModelSetup1Phase>();

  params.set<std::string>("fe_family") = "LAGRANGE";
  params.set<std::string>("fe_order") = "FIRST";

  return params;
}

JacobianTest1PhaseAction::JacobianTest1PhaseAction(InputParameters params)
  : JacobianTestAction(params), FlowModelSetup1Phase(params)
{
}

void
JacobianTest1PhaseAction::addInitialConditions()
{
  FlowModelSetup1Phase::addInitialConditions();
}

void
JacobianTest1PhaseAction::addSolutionVariables()
{
  FlowModelSetup1Phase::addSolutionVariables();
}

void
JacobianTest1PhaseAction::addNonConstantAuxVariables()
{
  FlowModelSetup1Phase::addNonConstantAuxVariables();
}

void
JacobianTest1PhaseAction::addMaterials()
{
  FlowModelSetup1Phase::addMaterials();
}

void
JacobianTest1PhaseAction::addUserObjects()
{
  FlowModelSetup1Phase::addUserObjects();
}
