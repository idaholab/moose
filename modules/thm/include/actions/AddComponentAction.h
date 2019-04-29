#pragma once

#include "THMObjectAction.h"

class AddComponentAction;

template <>
InputParameters validParams<AddComponentAction>();

class AddComponentAction : public THMObjectAction
{
public:
  AddComponentAction(InputParameters params);

  virtual void act();
};
