//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PeridynamicsKernelBase.h"

/**
 * Kernel class to implement hear source term for peridynamic heat conduction models
 */
class HeatSourceBPD : public PeridynamicsKernelBase
{
public:
  static InputParameters validParams();

  HeatSourceBPD(const InputParameters & parameters);

protected:
  virtual void computeLocalResidual() override;

  /// Power density function
  const Function & _power_density;
};
