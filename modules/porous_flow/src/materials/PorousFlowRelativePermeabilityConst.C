//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
