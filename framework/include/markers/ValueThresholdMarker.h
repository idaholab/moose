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

class ValueThresholdMarker : public QuadraturePointMarker
{
public:
  static InputParameters validParams();

  ValueThresholdMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeQpMarker() override;

  bool _coarsen_set;
  Real _coarsen;
  bool _refine_set;
  Real _refine;

  bool _invert;
};
