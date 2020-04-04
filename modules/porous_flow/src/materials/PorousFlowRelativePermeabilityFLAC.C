//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowRelativePermeabilityFLAC.h"

registerMooseObject("PorousFlowApp", PorousFlowRelativePermeabilityFLAC);

InputParameters
PorousFlowRelativePermeabilityFLAC::validParams()
{
  InputParameters params = PorousFlowRelativePermeabilityBase::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "m", "m >= 0", "relperm = (1 + m)seff^m - m seff^(m+1)");
  params.addClassDescription(
      "This Material calculates relative permeability of a phase using a model inspired by FLAC");
  return params;
}

PorousFlowRelativePermeabilityFLAC::PorousFlowRelativePermeabilityFLAC(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBase(parameters), _m(getParam<Real>("m"))
{
}

Real
PorousFlowRelativePermeabilityFLAC::relativePermeability(Real seff) const
{
  return PorousFlowFLACrelperm::relativePermeability(seff, _m);
}

Real
PorousFlowRelativePermeabilityFLAC::dRelativePermeability(Real seff) const
{
  return PorousFlowFLACrelperm::dRelativePermeability(seff, _m);
}
