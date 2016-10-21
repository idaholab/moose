/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityVG.h"

template<>
InputParameters validParams<PorousFlowRelativePermeabilityVG>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addRequiredRangeCheckedParam<Real>("m_j", "m_j > 0 & m_j < 1", "The van Genuchten exponent of phase j");
  params.addRangeCheckedParam<Real>("s_ls", 1.0, "s_ls > 0 & s_ls <= 1", "The fully saturated phase saturation. Default is 1");
  params.addClassDescription("This Material calculates relative permeability of a phase Sj using the van Genuchten model");
  return params;
}

PorousFlowRelativePermeabilityVG::PorousFlowRelativePermeabilityVG(const InputParameters & parameters) :
    PorousFlowRelativePermeabilityBase(parameters),
    _m(getParam<Real>("m_j")),
    _s_ls(getParam<Real>("s_ls"))
{
}

Real
PorousFlowRelativePermeabilityVG::effectiveSaturation(Real saturation) const
{
  return (saturation - _s_res) / (_s_ls - _s_res);
}

Real
PorousFlowRelativePermeabilityVG::relativePermeability(Real seff) const
{
  return std::sqrt(seff) * std::pow(1.0 - std::pow(1.0 - std::pow(seff, 1.0 / _m), _m), 2.0);
}

Real
PorousFlowRelativePermeabilityVG::dRelativePermeability_dS(Real seff) const
{
  // Guard against division by zero
  if (seff <= 0.0 || seff >= 1.0)
   return 0.0;

  Real a = 1.0 - std::pow(seff, 1.0 / _m);
  Real a2 = 1.0 - std::pow(a, _m);
  Real dkrel = (0.5 * std::pow(seff, -0.5) * a2 * a2 + 2.0 * std::pow(seff, 1.0 / _m - 0.5) *
    std::pow(a, _m - 1.0) * a2) / (_s_ls - _s_res);

  return dkrel;
}
