//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1PhaseWithAddedMass.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "HeatTransfer1PhaseBase.h"
#include "Closures1PhaseBase.h"
#include "ThermalHydraulicsApp.h"
#include "SlopeReconstruction1DInterface.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannel1PhaseWithAddedMass);

InputParameters
FlowChannel1PhaseWithAddedMass::validParams()
{
  InputParameters params = FlowChannel1Phase::validParams();
  return params;
}

FlowChannel1PhaseWithAddedMass::FlowChannel1PhaseWithAddedMass(const InputParameters & params)
  : FlowChannel1Phase(params)
{
}

void
FlowChannel1PhaseWithAddedMass::addMooseObjects()
{
  FlowChannel1Phase::addMooseObjects();

  std::string class_name = "BodyForce";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
  params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
  params.set<PostprocessorName>("postprocessor") = "mass_imbalance_source_pp";
  getTHMProblem().addKernel(
      class_name, genName(name(), FlowModelSinglePhase::RHOA, "rho_add_mass"), params);
}
