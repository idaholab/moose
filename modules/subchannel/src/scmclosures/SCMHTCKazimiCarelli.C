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
  params.addClassDescription(
      "Class that computes the convective heat transfer coefficient using the "
      "Kazimi-Carelli correlation. Only use for fuel-pins.");
  return params;
}

SCMHTCKazimiCarelli::SCMHTCKazimiCarelli(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
  // Check that Graber-Rieger is not used for the duct (not supported yet)
  if (const auto * duct_uo = _scm_problem->getDuctHTCClosure(); duct_uo && duct_uo == this)
    mooseError("'Graber-Rieger' is not yet supported for the 'duct_htc_correlation'.");
}

Real
SCMHTCKazimiCarelli::computeNusseltNumber(const FrictionStruct & /*friction_args*/,
                                          const NusseltStruct & nusselt_args) const
{
  const auto pre = computeNusseltNumberPreInfo(nusselt_args);
  const auto Pe = pre.Re * pre.Pr;

  if (Pe < 10 || Pe > 5000)
    flagSolutionWarning("Peclet number (Pe) out of range for the Kazimi-Carelli correlation.");

  if (pre.poD < 1.1 || pre.poD > 1.4)
    flagSolutionWarning(
        "Pitch over pin diameter ratio out of range for the Kazimi-Carelli correlation.");

  // Laminar regime
  if (pre.Re <= pre.ReL)
    return pre.laminar_Nu;

  // Blend transitional/turbulent using precomputed thresholds
  auto blended_Nu = [&](Real NuT) -> Real
  {
    if (pre.Re >= pre.ReT)
      return NuT;

    const auto denom = (pre.ReT - pre.ReL);
    const auto w = (pre.Re - pre.ReL) / denom;
    return w * NuT + (1.0 - w) * pre.laminar_Nu;
  };

  const auto NuT = 4.0 + 0.33 * std::pow(pre.poD, 3.8) * std::pow((Pe / 1e2), 0.86) +
                   0.16 * std::pow(pre.poD, 5);
  return blended_Nu(NuT);
}
