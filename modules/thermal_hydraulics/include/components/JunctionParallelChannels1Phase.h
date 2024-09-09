//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumeJunction1Phase.h"

/**
 * Junction between 1-phase flow channels that are parallel
 */
class JunctionParallelChannels1Phase : public VolumeJunction1Phase
{
public:
  JunctionParallelChannels1Phase(const InputParameters & params);

protected:
  /**
   * Builds user object for computing and storing the fluxes
   */
  virtual void buildVolumeJunctionUserObject() override;

public:
  static InputParameters validParams();
};
