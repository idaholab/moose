//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCZMComputeGlobalTractionBase.h"

/**
 * AD equivalent of CZMComputeGlobalTractionSmallStrain
 */
class ADCZMComputeGlobalTractionSmallStrain : public ADCZMComputeGlobalTractionBase
{
public:
  static InputParameters validParams();
  ADCZMComputeGlobalTractionSmallStrain(const InputParameters & parameters);

protected:
  void computeEquilibriumTracion() override;
};
