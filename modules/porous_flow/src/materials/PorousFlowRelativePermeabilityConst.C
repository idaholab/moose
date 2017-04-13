/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowRelativePermeabilityConst.h"

template <>
InputParameters
validParams<PorousFlowRelativePermeabilityConst>()
{
  InputParameters params = validParams<PorousFlowRelativePermeabilityBase>();
  params.addParam<Real>("kr", 1.0, "Relative permeability");
  params.addClassDescription(
      "This class sets the relative permeability to a constant value (default = 1)");
  return params;
}

PorousFlowRelativePermeabilityConst::PorousFlowRelativePermeabilityConst(
    const InputParameters & parameters)
  : PorousFlowRelativePermeabilityBase(parameters), _relperm(getParam<Real>("kr"))
{
}

Real PorousFlowRelativePermeabilityConst::relativePermeability(Real /*seff*/) const
{
  return _relperm;
}

Real PorousFlowRelativePermeabilityConst::dRelativePermeability(Real /*seff*/) const { return 0.0; }
