/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SinglePhaseFluidPropertiesPT.h"

template<>
InputParameters validParams<SinglePhaseFluidPropertiesPT>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidPropertiesPT::SinglePhaseFluidPropertiesPT(const InputParameters & parameters) :
    FluidProperties(parameters),
    _R(8.3144621),
    _T_c2k(273.15)
{
}

SinglePhaseFluidPropertiesPT::~SinglePhaseFluidPropertiesPT()
{
}

Real
SinglePhaseFluidPropertiesPT::gamma(Real pressure, Real temperature) const
{
  return cp(pressure, temperature) / cv(pressure, temperature);
}
