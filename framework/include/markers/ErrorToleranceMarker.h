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

class ErrorToleranceMarker : public IndicatorMarker
{
public:
  static InputParameters validParams();

  ErrorToleranceMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  Real _coarsen;
  Real _refine;
};
