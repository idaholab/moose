/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityFLAC.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityFLAC>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
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
