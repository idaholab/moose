#include "ClosureTest1PhaseAction.h"

registerMooseAction("THMTestApp", ClosureTest1PhaseAction, "meta_action");

InputParameters
ClosureTest1PhaseAction::validParams()
{
  InputParameters params = emptyInputParameters();
  params += ClosureTestAction::validParams();
  params += FlowModelSetup1Phase::validParams();

  return params;
}

ClosureTest1PhaseAction::ClosureTest1PhaseAction(InputParameters params)
  : ClosureTestAction(params), FlowModelSetup1Phase(params)
{
}

void
ClosureTest1PhaseAction::addInitialConditions()
{
  ClosureTestAction::addInitialConditions();
  FlowModelSetup1Phase::addInitialConditions();
}

void
ClosureTest1PhaseAction::addAuxVariables()
{
  ClosureTestAction::addAuxVariables();

  // add the usual solution variables as aux variables
  FlowModelSetup::addAuxVariable(_rhoA_name);
  FlowModelSetup::addAuxVariable(_rhouA_name);
  FlowModelSetup::addAuxVariable(_rhoEA_name);

  // add the normal aux variables
  FlowModelSetup1Phase::addNonConstantAuxVariables();
}

void
ClosureTest1PhaseAction::addMaterials()
{
  ClosureTestAction::addMaterials();
  FlowModelSetup1Phase::addMaterials();
}

void
ClosureTest1PhaseAction::addUserObjects()
{
  FlowModelSetup1Phase::addUserObjects();
}
