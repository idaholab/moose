/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityBC.h"
#include "PorousFlowBrooksCorey.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityBC>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
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
