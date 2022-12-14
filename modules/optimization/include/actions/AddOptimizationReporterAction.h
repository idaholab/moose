#pragma once

#include "MooseObjectAction.h"

class AddOptimizationReporterAction : public MooseObjectAction
{
public:
  static InputParameters validParams();
  AddOptimizationReporterAction(const InputParameters & params);

  virtual void act() override;
};
