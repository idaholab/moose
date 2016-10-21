/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityCorey.h"

template<>
InputParameters validParams<PorousFlowRelativePermeabilityCorey>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addRequiredParam<Real>("n_j", "The Corey exponent of phase j.");
  params.addClassDescription("This Material calculates relative permeability of either phase Sj, using the simple Corey model ((Sj-Sjr)/(1-S1r-S2r))^n");
  return params;
}

PorousFlowRelativePermeabilityCorey::PorousFlowRelativePermeabilityCorey(const InputParameters & parameters) :
    PorousFlowRelativePermeabilityBase(parameters),
    _n(getParam<Real>("n_j"))
{
}

Real
PorousFlowRelativePermeabilityCorey::relativePermeability(Real seff) const
{
  return std::pow(seff, _n);
}

Real
PorousFlowRelativePermeabilityCorey::dRelativePermeability_dS(Real seff) const
{
  return _n * std::pow(seff, _n - 1.0);
}
