/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MultiComponentFluidPropertiesPT.h"

template <>
InputParameters
validParams<MultiComponentFluidPropertiesPT>()
{
  InputParameters params = validParams<FluidProperties>();
  return params;
}

MultiComponentFluidPropertiesPT::MultiComponentFluidPropertiesPT(const InputParameters & parameters)
  : FluidProperties(parameters), _T_c2k(273.15)
{
}

MultiComponentFluidPropertiesPT::~MultiComponentFluidPropertiesPT() {}
