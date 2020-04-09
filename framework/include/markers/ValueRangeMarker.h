//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "QuadraturePointMarker.h"

class ValueRangeMarker : public QuadraturePointMarker
{
public:
  static InputParameters validParams();

  ValueRangeMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeQpMarker() override;

  Real _lower_bound;
  Real _upper_bound;
  Real _buffer_size;

  MarkerValue _inside;
  MarkerValue _outside;
};
