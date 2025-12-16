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
  // Enumerations
  MooseEnum factors("Presser Weisman none", "Presser");
  InputParameters params = SCMHTCClosureBase::validParams();
  params.addClassDescription(
      "Class that computes the convective heat transfer coefficient using the "
      "Dittus Boelter correlation.");
  params.addParam<MooseEnum>(
      "correction_factor",
      factors,
      "Correction factor modeling the effect of the fuel-pin bundle. Default is Presser");
  return params;
}

SCMHTCDittusBoelter::SCMHTCDittusBoelter(const InputParameters & parameters)
  : SCMHTCClosureBase(parameters),
    _is_tri_lattice(dynamic_cast<const TriSubChannelMesh *>(&_subchannel_mesh) != nullptr),
    _correction_factor(getParam<MooseEnum>("correction_factor"))
{
}

Real
SCMHTCDittusBoelter::computeNusseltNumber(const FrictionStruct & /*friction_args*/,
                                          const NusseltStruct & nusselt_args) const
{
  const auto pre = computeNusseltNumberPreInfo(nusselt_args);

  if (pre.Pr < 0.7 || pre.Pr > 1.6e2)
    flagSolutionWarning("Prandtl number (Pr) out of range for the Dittus-Boelter correlation.");

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

  const auto corr = computeCorrectionFactor(pre.poD);
  const Real psi = corr.psi;
  const Real b = corr.b;
  auto NuT = 0.023 * std::pow(pre.Re, 0.8) * std::pow(pre.Pr, b);
  NuT *= psi;

  return blended_Nu(NuT);
}

SCMHTCDittusBoelter::CorrectionResult
SCMHTCDittusBoelter::computeCorrectionFactor(const Real poD) const
{
  CorrectionResult result;

  switch (_correction_factor)
  {
    case 0: // Presser
    {
      result.b = 0.4;

      if (_is_tri_lattice)
      {
        if (poD < 1.05 || poD > 2.2)
          flagSolutionWarning("P/D out of range for Presser correction factor (triangular).");

        result.psi = 0.9090 + 0.0783 * poD - 0.1283 * std::exp(-2.4 * (poD - 1.0));
      }
      else
      {
        if (poD < 1.05 || poD > 1.9)
          flagSolutionWarning("P/D out of range for Presser correction factor (square).");

        result.psi = 0.9217 + 0.1478 * poD - 0.1130 * std::exp(-7.0 * (poD - 1.0));
      }
      return result;
    }

    case 1: // Weisman
    {
      result.b = 0.333;

      if (_is_tri_lattice)
      {
        if (poD < 1.1 || poD > 1.5)
          flagSolutionWarning("P/D out of range for Weisman correction factor (triangular).");

        result.psi = 1.130 * poD - 0.2609;
      }
      else
      {
        if (poD < 1.1 || poD > 1.3)
          flagSolutionWarning("P/D out of range for Weisman correction factor (square).");

        result.psi = 1.826 * poD - 1.0430;
      }
      return result;
    }

    case 2: // "none": no bundle correction, keep classical Dittus-Boelter exponent (b = 0.4)
    default:
    {
      result.psi = 1.0;
      result.b = 0.4;
      return result;
    }
  }
}
