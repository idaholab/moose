//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCSchadModified.h"

registerMooseObject("SubChannelApp", SCMHTCSchadModified);

InputParameters
SCMHTCSchadModified::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription("Class that models the convective heat transfer coefficient using the "
                             "Schad-Modified correlation. Only use for fuel-pins.");
  return params;
}

SCMHTCSchadModified::SCMHTCSchadModified(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
}

Real
SCMHTCSchadModified::computeNusseltNumber(const FrictionStruct & /*friction_args*/,
                                          const NusseltStruct & nusselt_args) const
{
  // Check that the correlation not used for the duct (not supported yet)
  if (const auto * duct_uo = _scm_problem->getDuctHTCClosure(); duct_uo && duct_uo == this)
    mooseError("'Schad-Modified' is not yet supported for the 'duct_htc_correlation'.");

  const auto pre = computeNusseltNumberPreInfo(nusselt_args);

  const auto Pe = pre.Re * pre.Pr;

  if (pre.poD < 1.1 || pre.poD > 1.5)
    flagSolutionWarning(
        "Pitch over pin diameter ratio out of range for the Schad-Modified correlation.");

  const Real poly = -16.15 + 24.96 * pre.poD - 8.55 * Utility::pow<2>(pre.poD);
  auto NuT = 0.0;
  if (Pe <= 1000 && Pe >= 150)
  {
    NuT = poly * std::pow(Pe, 0.3);
  }
  else if (Pe < 150)
  {
    NuT = poly * 4.496;
    flagSolutionWarning(
        "Peclet number (Pe) below recommended range for the Schad-Modified correlation.");
  }
  else
  {
    flagSolutionWarning(
        "Peclet number (Pe) above recommended range for the Schad-Modified correlation.");
    NuT = poly * std::pow(Pe, 0.3);
  }

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

  return blended_Nu(NuT);
}
