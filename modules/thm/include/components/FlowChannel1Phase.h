#ifndef FLOWCHANNEL1PHASE_H
#define FLOWCHANNEL1PHASE_H

#include "FlowChannelBase.h"

class FlowChannel1Phase;
class ClosuresBase;

template <>
InputParameters validParams<FlowChannel1Phase>();

/**
 * A class representing a 1-phase flow channel
 *
 * A flow channel is defined by its position, direction, length and area.
 */
class FlowChannel1Phase : public FlowChannelBase
{
public:
  FlowChannel1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

  /**
   * Gets 1-phase wall heat transfer coefficient names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHTCNames1Phase() const { return _Hw_1phase_names; }

protected:
  virtual std::shared_ptr<FlowModel> buildFlowModel() override;
  virtual void check() const override;

  virtual void addFormLossObjects();

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames() override;

  /// 1-phase wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_1phase_names;
};

#endif /* FLOWCHANNEL1PHASE_H */
