/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityCorey.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityCorey>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addRequiredParam<Real>("n", "The Corey exponent of the phase.");
  params.addClassDescription("This Material calculates relative permeability of the fluid phase, "
                             "using the simple Corey model ((S-S_res)/(1-sum(S_res)))^n");
  return params;
}

PorousFlowRelativePermeabilityCorey::PorousFlowRelativePermeabilityCorey(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBase(parameters), _n(getParam<Real>("n"))
{
}

Real
PorousFlowRelativePermeabilityCorey::relativePermeability(Real seff) const
{
  return std::pow(seff, _n);
}

Real
PorousFlowRelativePermeabilityCorey::dRelativePermeability(Real seff) const
{
  return _n * std::pow(seff, _n - 1.0);
}
