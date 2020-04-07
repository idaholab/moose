#pragma once

#include "FlowChannelBase.h"

class ClosuresBase;

/**
 * A class representing a 1-phase flow channel
 *
 * A flow channel is defined by its position, direction, length and area.
 */
class FlowChannel1Phase : public FlowChannelBase
{
public:
  FlowChannel1Phase(const InputParameters & params);

  /**
   * Gets 1-phase wall heat transfer coefficient names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHTCNames1Phase() const { return _Hw_1phase_names; }

  virtual const THM::FlowModelID & getFlowModelID() const override { return THM::FM_SINGLE_PHASE; }

protected:
  virtual void init() override;
  virtual std::shared_ptr<FlowModel> buildFlowModel() override;
  virtual void check() const override;

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames() override;

  /// 1-phase wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_1phase_names;

public:
  static InputParameters validParams();
};
