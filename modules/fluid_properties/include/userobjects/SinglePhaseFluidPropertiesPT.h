//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SinglePhaseFluidProperties.h"

class SinglePhaseFluidPropertiesPT;

template <>
InputParameters validParams<SinglePhaseFluidPropertiesPT>();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Common class for single phase fluid properties using a pressure
 * and temperature formulation
 */
class SinglePhaseFluidPropertiesPT : public SinglePhaseFluidProperties
{
public:
  SinglePhaseFluidPropertiesPT(const InputParameters & parameters);
  virtual ~SinglePhaseFluidPropertiesPT();
};

#pragma GCC diagnostic pop

