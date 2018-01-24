//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
