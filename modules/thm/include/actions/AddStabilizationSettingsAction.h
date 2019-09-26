#pragma once

#include "MooseObjectAction.h"

class AddStabilizationSettingsAction;

template <>
InputParameters validParams<AddStabilizationSettingsAction>();

class AddStabilizationSettingsAction : public MooseObjectAction
{
public:
  AddStabilizationSettingsAction(InputParameters params);

  void act();
};
