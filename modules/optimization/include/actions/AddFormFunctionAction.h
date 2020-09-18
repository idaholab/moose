#pragma once

#include "MooseObjectAction.h"

class AddFormFunctionAction : public MooseObjectAction
{
public:
  static InputParameters validParams();
  AddFormFunctionAction(InputParameters params);

  virtual void act() override;
};
