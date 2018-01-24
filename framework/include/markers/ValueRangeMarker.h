//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VALUERANGEMARKER_H
#define VALUERANGEMARKER_H

#include "QuadraturePointMarker.h"

class ValueRangeMarker;

template <>
InputParameters validParams<ValueRangeMarker>();

class ValueRangeMarker : public QuadraturePointMarker
{
public:
  ValueRangeMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeQpMarker() override;

  Real _lower_bound;
  Real _upper_bound;
  Real _buffer_size;

  MarkerValue _third_state;
  MarkerValue _inside;
  MarkerValue _outside;

  const VariableValue & _u;
};

#endif /* VALUERANGEMARKER_H */
