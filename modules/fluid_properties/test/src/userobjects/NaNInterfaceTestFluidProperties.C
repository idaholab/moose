//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaNInterfaceTestFluidProperties.h"

registerMooseObject("FluidPropertiesTestApp", NaNInterfaceTestFluidProperties);

template <>
InputParameters
validParams<NaNInterfaceTestFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();
  params += validParams<NaNInterface>();

  params.addClassDescription("Fluid properties for testing NaNInterface");

  return params;
}

NaNInterfaceTestFluidProperties::NaNInterfaceTestFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), NaNInterface(this)
{
}

Real
NaNInterfaceTestFluidProperties::p_from_v_e(Real, Real) const
{
  return getNaN();
}

void
NaNInterfaceTestFluidProperties::p_from_v_e(Real, Real, Real &, Real &, Real &) const
{
}
