//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

class LowerDIntegralSideUserObject : public SideUserObject
{
public:
  static InputParameters validParams();
  LowerDIntegralSideUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  const VariableValue * _lower_d_value;

  Real _integral_value;
};
