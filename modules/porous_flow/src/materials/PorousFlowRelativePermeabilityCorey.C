//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityCorey.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityCorey);

InputParameters
PorousFlowRelativePermeabilityCorey::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBase::validParams();
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
