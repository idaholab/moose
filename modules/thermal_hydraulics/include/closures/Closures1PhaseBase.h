//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ClosuresBase.h"

class FlowChannel1Phase;

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
   * This function assumes that the flow channel component has the parameter
   * 'f' as a valid parameter, so this function should be guarded appropriately.
   *
   * @param[in] flow_channel   Flow channel component
   */
  void addWallFrictionFunctionMaterial(const FlowChannel1Phase & flow_channel) const;

  /**
   * Adds average wall temperature material
   *
   * @param[in] flow_channel   Flow channel component
   */
  void addAverageWallTemperatureMaterial(const FlowChannel1Phase & flow_channel) const;

public:
  static InputParameters validParams();
};
