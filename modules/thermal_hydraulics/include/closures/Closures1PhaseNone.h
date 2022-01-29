#pragma once

#include "Closures1PhaseBase.h"

/**
 * Sets up no 1-phase closures
 *
 * Users have to implement the closure materials and supply them in the input file
 */
class Closures1PhaseNone : public Closures1PhaseBase
{
public:
  Closures1PhaseNone(const InputParameters & params);

  virtual void checkFlowChannel(const FlowChannelBase & flow_channel) const override;
  virtual void checkHeatTransfer(const HeatTransferBase & heat_transfer,
                                 const FlowChannelBase & flow_channel) const override;
  virtual void addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel) override;
  virtual void addMooseObjectsHeatTransfer(const HeatTransferBase & heat_transfer,
                                           const FlowChannelBase & flow_channel) override;

public:
  static InputParameters validParams();
};
