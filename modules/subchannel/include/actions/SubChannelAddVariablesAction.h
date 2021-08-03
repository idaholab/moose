#pragma once

#include "Action.h"
#include "MooseEnum.h"

/**
 * Action that adds SubChannel variables needs for the solve
 */
class SubChannelAddVariablesAction : public Action
{
public:
  static InputParameters validParams();

  SubChannelAddVariablesAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /// FE family of the aux variables added by this action
  MooseEnum _fe_family;
  /// FE order of the aux variables added by this action
  MooseEnum _fe_order;
};
