//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSFVHLLCBCBase.h"

class NSFVHLLCSourceBC : public CNSFVHLLCBCBase
{
public:
  NSFVHLLCSourceBC(const InputParameters & parameters);

  static InputParameters validParams();

  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;

protected:
  void preComputeWaveSpeed() override;
  ADReal computeQpResidual() override { mooseError("Don't call me."); }

  virtual ADReal sourceElem() = 0;
};
