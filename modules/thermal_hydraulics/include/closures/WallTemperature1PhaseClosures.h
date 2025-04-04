//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Closures1PhaseBase.h"

/**
 * Adds wall temperature material for single-phase flow.
 */
class WallTemperature1PhaseClosures : public Closures1PhaseBase
{
public:
  static InputParameters validParams();

  WallTemperature1PhaseClosures(const InputParameters & params);

  virtual void checkFlowChannel(const FlowChannelBase & flow_channel) const override;
  virtual void checkHeatTransfer(const HeatTransferBase & heat_transfer,
                                 const FlowChannelBase & flow_channel) const override;
  virtual void addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel) override;
  virtual void addMooseObjectsHeatTransfer(const HeatTransferBase & heat_transfer,
                                           const FlowChannelBase & flow_channel) override;
};
