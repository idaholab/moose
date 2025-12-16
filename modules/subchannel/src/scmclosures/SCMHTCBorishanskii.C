//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCBorishanskii.h"

registerMooseObject("SubChannelApp", SCMHTCBorishanskii);

InputParameters
SCMHTCBorishanskii::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription(
      "Class that computes the convective heat transfer coefficient using the "
      "Borishanskii correlation. Only use for fuel-pins.");
  return params;
}

SCMHTCBorishanskii::SCMHTCBorishanskii(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
  // Check that the correlation is not used for the duct (not supported yet)
  if (const auto * duct_uo = _scm_problem->getDuctHTCClosure(); duct_uo && duct_uo == this)
    mooseError("'Borishanskii' is not yet supported for the 'duct_htc_correlation'.");
}

Real
SCMHTCBorishanskii::computeNusseltNumber(const FrictionStruct & /*friction_args*/,
                                         const NusseltStruct & nusselt_args) const
{
  const auto pre = computeNusseltNumberPreInfo(nusselt_args);
  const Real Pe = pre.Re * pre.Pr;

  if (pre.poD < 1.1 || pre.poD > 1.5)
    flagSolutionWarning(
        "Pitch-over-pin diameter ratio out of range for the Borishanskii correlation.");

  // Base Borishanskii correlation term
  const Real poly = -8.12 + 12.76 * pre.poD - 3.65 * Utility::pow<2>(pre.poD);
  if (poly <= 0.0)
  {
    mooseError("Logarithm argument non-positive in Borishanskii correlation; "
               "Check Pitch-over-pin diameter ratio.");
  }

  auto NuT = 24.15 * std::log(poly);
  // Peclet number correction term
  const Real corr_prefactor = 0.0174 * (1.0 - std::exp(6.0 - 6.0 * pre.poD));

  if (Pe >= 200.0 && Pe <= 2200.0)
  {
    NuT += corr_prefactor * std::pow(Pe - 200.0, 0.9);
  }
  else if (Pe < 200.0)
  {
    // do nothing, no extra term
  }
  else // Pe > 2200
  {
    flagSolutionWarning(
        "Peclet number (Pe) above recommended range for the Borishanskii correlation.");
    // Still apply the same correlation formula, but with warning
    NuT += corr_prefactor * std::pow(Pe - 200.0, 0.9);
  }

  // Laminar regime
  if (pre.Re <= pre.ReL)
    return pre.laminar_Nu;

  // Blend transitional/turbulent using precomputed thresholds
  auto blended_Nu = [&](Real Nu_turb) -> Real
  {
    if (pre.Re >= pre.ReT)
      return Nu_turb;

    const Real denom = pre.ReT - pre.ReL;
    const auto w = (pre.Re - pre.ReL) / denom;
    return w * Nu_turb + (1.0 - w) * pre.laminar_Nu;
  };

  return blended_Nu(NuT);
}
