//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCGraberRieger.h"

registerMooseObject("SubChannelApp", SCMHTCGraberRieger);

InputParameters
SCMHTCGraberRieger::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription("Class that models the convective heat transfer coefficient using the "
                             "Graber-Rieger correlation. Only use for fuel-pins.");
  return params;
}

SCMHTCGraberRieger::SCMHTCGraberRieger(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
}

Real
SCMHTCGraberRieger::computeNusseltNumber(const FrictionStruct & /*friction_args*/,
                                         const NusseltStruct & nusselt_args) const
{
  // Check that Graber-Rieger is not used for the duct (not supported yet)
  if (const auto * duct_uo = _scm_problem->getDuctHTCClosure(); duct_uo && duct_uo == this)
    mooseError("'Graber-Rieger' is not yet supported for the 'duct_htc_correlation'.");

  const auto pre = computeNusseltNumberPreInfo(nusselt_args);

  const auto Pe = pre.Re * pre.Pr;

  if (Pe < 150 || Pe > 3000)
    flagSolutionWarning("Pe number out of range for the Graber-Rieger correlation.");

  if (pre.poD < 1.25 || pre.poD > 1.95)
    flagSolutionWarning(
        "Pitch over pin diameter ratio out of range for the Graber-Rieger correlation.");

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

  const auto NuT =
      0.25 + 6.2 * pre.poD + (-0.007 + 0.032 * pre.poD) * std::pow(Pe, 0.8 - 0.024 * pre.poD);
  return blended_Nu(NuT);
}
