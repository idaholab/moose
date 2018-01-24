//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AREAPOSTPROCESSOR_H
#define AREAPOSTPROCESSOR_H

#include "SideIntegralPostprocessor.h"

// Forward Declarations
class AreaPostprocessor;

template <>
InputParameters validParams<AreaPostprocessor>();

/**
 * This postprocessor computes the area of a specified block.
 */
class AreaPostprocessor : public SideIntegralPostprocessor
{
public:
  AreaPostprocessor(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;
};

#endif
