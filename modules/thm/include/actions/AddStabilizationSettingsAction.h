#ifndef ADDSTABILIZATIONSETTINGSACTION_H
#define ADDSTABILIZATIONSETTINGSACTION_H

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

#endif /* ADDSTABILIZATIONSETTINGSACTION_H */
