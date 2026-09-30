//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ClosuresBase.h"

/**
 * Base class for 1-phase closures
 */
class Closures1PhaseBase : public ClosuresBase
{
public:
  Closures1PhaseBase(const InputParameters & params);

protected:
  /**
   * Adds material that computes wall friction factor from a specified function
   *
   * This function assumes that the flow channel has a wall friction factor function ('f'), so
   * this function should be guarded appropriately.
   *
   * @param[in] flow_channel   Flow channel
   */
  void addWallFrictionFunctionMaterial(const FlowChannelClosuresInterface & flow_channel) const;

  /**
   * Adds average wall temperature material
   *
   * @param[in] flow_channel   Flow channel
   */
  void addAverageWallTemperatureMaterial(const FlowChannelClosuresInterface & flow_channel) const;

public:
  static InputParameters validParams();
};
