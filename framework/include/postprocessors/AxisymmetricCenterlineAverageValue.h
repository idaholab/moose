/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
