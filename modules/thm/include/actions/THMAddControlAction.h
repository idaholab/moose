#pragma once

#include "AddControlAction.h"

class THMAddControlAction;

template <>
InputParameters validParams<THMAddControlAction>();

/**
 * Action for adding THM control objects
 */
class THMAddControlAction : public AddControlAction
{
public:
  /**
   * Class constructor
   * @param params Parameters for this Action
   */
  THMAddControlAction(InputParameters parameters);

  virtual void act() override;
};
