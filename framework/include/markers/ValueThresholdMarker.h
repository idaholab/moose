//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VALUETHRESHOLDMARKER_H
#define VALUETHRESHOLDMARKER_H

#include "QuadraturePointMarker.h"

class ValueThresholdMarker;

template <>
InputParameters validParams<ValueThresholdMarker>();

class ValueThresholdMarker : public QuadraturePointMarker
{
public:
  ValueThresholdMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeQpMarker() override;

  bool _coarsen_set;
  Real _coarsen;
  bool _refine_set;
  Real _refine;

  bool _invert;
  MarkerValue _third_state;

  const VariableValue & _u;
};

#endif /* VALUETHRESHOLDMARKER_H */
