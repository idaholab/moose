#pragma once

#include "MooseObjectAction.h"

class AddComponentAction : public MooseObjectAction
{
public:
  AddComponentAction(InputParameters params);

  virtual void act() override;

protected:
  /// True if building a component group
  bool _group;

public:
  static InputParameters validParams();
};
