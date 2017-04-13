/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<SinglePhaseFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidProperties::SinglePhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties() {}

Real
SinglePhaseFluidProperties::gamma(Real v, Real u) const
{
  return cp(v, u) / cv(v, u);
}
