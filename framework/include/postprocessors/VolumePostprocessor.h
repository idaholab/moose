//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VOLUMEPOSTPROCESSOR_H
#define VOLUMEPOSTPROCESSOR_H

#include "ElementIntegralPostprocessor.h"

// Forward Declarations
class VolumePostprocessor;

template <>
InputParameters validParams<VolumePostprocessor>();

/**
 * This postprocessor computes the volume of a specified block.
 */
class VolumePostprocessor : public ElementIntegralPostprocessor
{
public:
  VolumePostprocessor(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;
};

#endif
