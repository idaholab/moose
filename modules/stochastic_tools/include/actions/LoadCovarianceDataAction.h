//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include "Action.h"
class GaussianProcessSurrogate;

class LoadCovarianceDataAction : public Action
{
public:
  static InputParameters validParams();
  LoadCovarianceDataAction(const InputParameters & params);
  virtual void act() override;

private:
  void load(GaussianProcessSurrogate & model);
};

#endif
