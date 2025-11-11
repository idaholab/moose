//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCDittusBoelter.h"

registerMooseObject("SubChannelApp", SCMHTCDittusBoelter);

InputParameters
SCMHTCDittusBoelter::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription(
      "Class that models the heat transfer coefficient using the Dittus Boelter correlation.");
  return params;
}

SCMHTCDittusBoelter::SCMHTCDittusBoelter(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
}

Real
SCMHTCDittusBoelter::computeNusseltNumber(const FrictionStruct & friction_args,
                                          const NusseltStruct & nusselt_args) const
{
  (void)friction_args; // silence unused parameter

  const auto pre = computeNusseltNumberPreInfo(nusselt_args);

  if (pre.Pr < 0.7 || pre.Pr > 1.6e2)
    mooseDoOnce(mooseWarning("Pr number out of range in the Dittus-Boelter correlation for "
                             "pin or duct surface temperature calculation."));

  // Laminar regime
  if (pre.Re <= pre.ReL)
    return pre.laminar_Nu;

  // Blend transitional/turbulent using precomputed thresholds
  auto blended_Nu = [&](Real NuT) -> Real
  {
    if (pre.Re >= pre.ReT)
      return NuT;

    const auto denom = (pre.ReT - pre.ReL);
    const auto w = denom > 0 ? (pre.Re - pre.ReL) / denom : 1.0; // guard against degenerate case
    return w * NuT + (1.0 - w) * pre.laminar_Nu;
  };

  const auto NuT = 0.023 * std::pow(pre.Re, 0.8) * std::pow(pre.Pr, 0.4);
  return blended_Nu(NuT);
}

Real
SCMHTCDittusBoelter::computeHTC(const FrictionStruct & friction_args,
                                const NusseltStruct & nusselt_args,
                                const Real & k) const
{
  // Compute HTC
  auto Nu = computeNusseltNumber(friction_args, nusselt_args);
  auto Dh_i = 4.0 * friction_args.S / friction_args.w_perim;
  return Nu * k / Dh_i;
}
