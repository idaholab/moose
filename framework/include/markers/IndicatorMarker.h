//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INDICATORMARKER_H
#define INDICATORMARKER_H

#include "Marker.h"

class IndicatorMarker;

template <>
InputParameters validParams<IndicatorMarker>();

class IndicatorMarker : public Marker
{
public:
  IndicatorMarker(const InputParameters & parameters);

protected:
  ErrorVector & _error_vector;
};

#endif /* INDICATORMARKER_H */
