#pragma once

#include "THMObjectAction.h"

class AddStabilizationSettingsAction;

template <>
InputParameters validParams<AddStabilizationSettingsAction>();

class AddStabilizationSettingsAction : public THMObjectAction
{
public:
  AddStabilizationSettingsAction(InputParameters params);

  void act();
};
