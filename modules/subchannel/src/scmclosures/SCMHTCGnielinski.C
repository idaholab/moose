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

  Real f_turb = _scm_problem.getFrictionClosure()->computeFrictionFactor(friction_args) / 8.0;

  /// Pr -> Pr + 0.01. We start flattening out the Nusselt profile in the correlation,
  /// which is what we should see in practice, i.e., for very low Pr numbers the heat exchange
  /// will be dominated by conduction and Nu profile should be flat.
  const auto Pr_shifted = pre.Pr + 0.01;
  auto denominator = 1 + 12.7 * std::sqrt(f_turb) * (std::pow(Pr_shifted, 2. / 3.) - 1.);
  // At very low Pr, (Pr_shifted)^(2/3) - 1 approaches -1, so once the friction factor is large
  // enough (routinely the case near the laminar-turbulent transition) this denominator can
  // cross zero and make Nu blow up or flip sign. Floor it so Nu instead saturates near the
  // flat, conduction-dominated behavior the low-Pr flattening above is meant to produce.
  if (denominator < 0.1)
  {
    flagSolutionWarning("Gnielinski correlation denominator is non-positive or near-singular "
                        "for the current Prandtl number and friction factor; flooring it to "
                        "keep the Nusselt number bounded.");
    denominator = 0.1;
  }
  const auto NuT = f_turb * (pre.Re - 1e3) * Pr_shifted / denominator;
  return blendTurbulentNusseltNumber(pre, NuT);
}
