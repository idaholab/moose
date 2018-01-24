//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AXISYMMETRICCENTERLINEAVERAGEVALUE_H
#define AXISYMMETRICCENTERLINEAVERAGEVALUE_H

#include "SideAverageValue.h"

// Forward Declarations
class AxisymmetricCenterlineAverageValue;

template <>
InputParameters validParams<AxisymmetricCenterlineAverageValue>();

/**
 * This postprocessor computes a line integral of the specified variable
 * along the centerline of an axisymmetric domain.
 */
class AxisymmetricCenterlineAverageValue : public SideAverageValue
{
public:
  AxisymmetricCenterlineAverageValue(const InputParameters & parameters);

protected:
  virtual Real computeIntegral() override;
  virtual Real volume() override;
  Real _volume;
};

#endif
