//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

class ReadExecutorParamsAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  ReadExecutorParamsAction(const InputParameters & params);

  virtual void act() override;

protected:
  virtual void setupAutoPreconditioning();

private:
  /// Whether to automatically add a preconditioner
  const bool _auto_preconditioning;
};
