//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannel1Phase);

InputParameters
FlowChannel1Phase::validParams()
{
  InputParameters params = FlowChannel1PhaseBase::validParams();

  std::vector<Real> sf_1phase(3, 1.0);
  params.addParam<std::vector<Real>>(
      "scaling_factor_1phase",
      sf_1phase,
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");

  params.addParamNamesToGroup("scaling_factor_1phase", "Numerical scheme");
  params.addClassDescription("1-phase 1D flow channel");

  return params;
}

FlowChannel1Phase::FlowChannel1Phase(const InputParameters & params) : FlowChannel1PhaseBase(params)
{
}

void
FlowChannel1Phase::checkFluidProperties() const
{
  const UserObject & fp = getTHMProblem().getUserObject<UserObject>(_fp_name);
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) == nullptr)
    logError("Supplied fluid properties must be for 1-phase fluids.");
}

std::string
FlowChannel1Phase::flowModelClassName() const
{
  return "FlowModelSinglePhase";
}

std::vector<std::string>
FlowChannel1Phase::ICParameters() const
{
  return {"initial_p", "initial_T", "initial_vel"};
}
