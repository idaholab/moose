//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IndicatorMarker.h"

class ErrorFractionMarker : public IndicatorMarker
{
public:
  static InputParameters validParams();

  ErrorFractionMarker(const InputParameters & parameters);

  virtual void markerSetup() override;

protected:
  virtual MarkerValue computeElementMarker() override;

  Real _coarsen;
  Real _refine;
  bool _clear_extremes;

  Real _max;
  Real _min;
  Real _delta;
  Real _refine_cutoff;
  Real _coarsen_cutoff;
};
