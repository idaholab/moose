//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannelGasMix.h"
#include "VaporMixtureFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannelGasMix);

InputParameters
FlowChannelGasMix::validParams()
{
  InputParameters params = FlowChannel1PhaseBase::validParams();

  params.addParam<FunctionName>("initial_mass_fraction",
                                "Initial mass fraction of the secondary gas");
  params.addParamNamesToGroup("initial_mass_fraction", "Variable initialization");

  params.addParam<Real>(
      "scaling_factor_xirhoA", 1.0, "Scaling factor for the secondary component mass equation");
  params.addParam<Real>("scaling_factor_rhoA", 1.0, "Scaling factor for the mixture mass equation");
  params.addParam<Real>("scaling_factor_rhouA", 1.0, "Scaling factor for the momentum equation");
  params.addParam<Real>("scaling_factor_rhoEA", 1.0, "Scaling factor for the energy equation");
  params.addParamNamesToGroup(
      "scaling_factor_xirhoA scaling_factor_rhoA scaling_factor_rhouA scaling_factor_rhoEA",
      "Numerical scheme");

  params.addClassDescription("Single-phase flow channel with a binary gas mixture");

  return params;
}

FlowChannelGasMix::FlowChannelGasMix(const InputParameters & params) : FlowChannel1PhaseBase(params)
{
}

void
FlowChannelGasMix::checkFluidProperties() const
{
  const UserObject & fp = getTHMProblem().getUserObject<UserObject>(_fp_name);
  if (dynamic_cast<const VaporMixtureFluidProperties *>(&fp) == nullptr)
    logError("The supplied fluid properties object must be of type "
             "'VaporMixtureFluidProperties'.");
}

std::string
FlowChannelGasMix::flowModelClassName() const
{
  return "FlowModelGasMix";
}

std::vector<std::string>
FlowChannelGasMix::ICParameters() const
{
  return {"initial_p", "initial_T", "initial_vel", "initial_mass_fraction"};
}
