#pragma once

#include "MooseObjectAction.h"

class AddComponentAction;

template <>
InputParameters validParams<AddComponentAction>();

class AddComponentAction : public MooseObjectAction
{
public:
  AddComponentAction(InputParameters params);

  virtual void act() override;

protected:
  /// True if building a component group
  bool _group;
};
