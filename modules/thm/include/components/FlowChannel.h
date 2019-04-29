#pragma once

#include "FlowChannelBase.h"

class FlowChannel;

template <>
InputParameters validParams<FlowChannel>();

/**
 * Deprecated class, do not use.
 */
class FlowChannel : public FlowChannelBase
{
public:
  FlowChannel(const InputParameters & params);

protected:
  virtual std::shared_ptr<FlowModel> buildFlowModel() override;
  virtual void check() const override;
};
