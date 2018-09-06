//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidPropertiesPT.h"

template <>
InputParameters
validParams<SinglePhaseFluidPropertiesPT>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();

  return params;
}

SinglePhaseFluidPropertiesPT::SinglePhaseFluidPropertiesPT(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
  mooseDeprecated(name(),
                  ": Your fluid property class inherits from SinglePhaseFluidPropertiesPT which is "
                  "now depreacted. Please, update your code so that your class inherits from "
                  "SinglePhaseFluidProperties.");
}

SinglePhaseFluidPropertiesPT::~SinglePhaseFluidPropertiesPT() {}
