//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityBC.h"
#include "PorousFlowBrooksCorey.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityBC);

InputParameters
PorousFlowRelativePermeabilityBC::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBase::validParams();
  params.addRequiredParam<Real>("lambda", "The Brooks-Corey exponent of the phase");
  params.addParam<bool>("nw_phase", false, "Set true if this is the non-wetting phase");
  params.addClassDescription("Brooks-Corey relative permeability");
  return params;
}

PorousFlowRelativePermeabilityBC::PorousFlowRelativePermeabilityBC(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBase(parameters),
    _lambda(getParam<Real>("lambda")),
    _is_nonwetting(getParam<bool>("nw_phase"))
{
}

Real
PorousFlowRelativePermeabilityBC::relativePermeability(Real seff) const
{
  if (_is_nonwetting)
    return PorousFlowBrooksCorey::relativePermeabilityNW(seff, _lambda);
  else
    return PorousFlowBrooksCorey::relativePermeabilityW(seff, _lambda);
}

Real
PorousFlowRelativePermeabilityBC::dRelativePermeability(Real seff) const
{
  if (_is_nonwetting)
    return PorousFlowBrooksCorey::dRelativePermeabilityNW(seff, _lambda);
  else
    return PorousFlowBrooksCorey::dRelativePermeabilityW(seff, _lambda);
}
