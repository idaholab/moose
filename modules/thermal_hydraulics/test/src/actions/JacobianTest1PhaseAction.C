//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JacobianTest1PhaseAction.h"

registerMooseAction("ThermalHydraulicsTestApp", JacobianTest1PhaseAction, "meta_action");

InputParameters
JacobianTest1PhaseAction::validParams()
{
  InputParameters params = emptyInputParameters();
  params += JacobianTestAction::validParams();
  params += FlowModelSetup1Phase::validParams();

  params.set<std::string>("fe_family") = "LAGRANGE";
  params.set<std::string>("fe_order") = "FIRST";

  return params;
}

JacobianTest1PhaseAction::JacobianTest1PhaseAction(const InputParameters & params)
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
JacobianTest1PhaseAction::addAuxVariables()
{
  TestAction::addAuxVariables();
  FlowModelSetup1Phase::addNonConstantAuxVariables();
}

void
JacobianTest1PhaseAction::addMaterials()
{
  TestAction::addMaterials();
  FlowModelSetup1Phase::addMaterials();
}

void
JacobianTest1PhaseAction::addUserObjects()
{
  FlowModelSetup1Phase::addUserObjects();
}
