//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCKazimiCarelli.h"

registerMooseObject("SubChannelApp", SCMHTCKazimiCarelli);

InputParameters
SCMHTCKazimiCarelli::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription("Class that models the heat transfer coefficient using the "
                             "Kazimi-Carelli correlation. Only use for fuel-pins.");
  return params;
}

SCMHTCKazimiCarelli::SCMHTCKazimiCarelli(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
}

Real
SCMHTCKazimiCarelli::computeNusseltNumber(const FrictionStruct & /*friction_args*/,
                                          const NusseltStruct & nusselt_args) const
{
  // Check that kazimi-carelli is not used for the duct (not supported yet)
  if (const auto * duct_uo = _scm_problem->getDuctHTCClosure(); duct_uo && duct_uo == this)
    mooseError("'kazimi-carelli' is not yet supported for the 'duct_htc_correlation'.");

  const auto pre = computeNusseltNumberPreInfo(nusselt_args);

  const auto Pe = pre.Re * pre.Pr;

  if (Pe < 10 || Pe > 5000)
    flagSolutionWarning("Pe number out of range in the Kazimi Carelli correlation for "
                        "pin or duct surface temperture calculation.");

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

  const auto NuT = 4.0 + 0.33 * std::pow(pre.poD, 3.8) * std::pow((Pe / 1e2), 0.86) +
                   0.16 * std::pow(pre.poD, 5);
  return blended_Nu(NuT);
}
