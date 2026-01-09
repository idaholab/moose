//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMHTCGnielinski.h"
#include "SCMFrictionClosureBase.h"

registerMooseObject("SubChannelApp", SCMHTCGnielinski);

InputParameters
SCMHTCGnielinski::validParams()
{
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription(
      "Class that computes the convective heat transfer coefficient using the "
      "Gnielinski correlation.");
  return params;
}

SCMHTCGnielinski::SCMHTCGnielinski(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters)
{
}

Real
SCMHTCGnielinski::computeNusseltNumber(const FrictionStruct & friction_args,
                                       const NusseltStruct & nusselt_args) const
{
  const auto pre = computeNusseltNumberPreInfo(nusselt_args);

  if (pre.Pr < 1e-5 || pre.Pr > 2e3)
    flagSolutionWarning("Prandtl number (Pr) out of range for the Gnielinski correlation.");

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

  Real f_turb = _scm_problem.getFrictionClosure()->computeFrictionFactor(friction_args) / 8.0;

  /// Pr -> Pr + 0.01. We start flattening out the Nusselt profile in the correlation,
  /// which is what we should see in practice, i.e., for very low Pr numbers the heat exchange
  /// will be dominated by conduction and Nu profile should be flat.
  const auto NuT = f_turb * (pre.Re - 1e3) * (pre.Pr + 0.01) /
                   (1 + 12.7 * std::sqrt(f_turb) * (std::pow(pre.Pr + 0.01, 2. / 3.) - 1.));
  return blended_Nu(NuT);
}
