//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCZMComputeLocalTractionTotalBase.h"

/**
 * Implementation of the purely elastic traction-separation law. This is the AD equivalent of
 *`PureElasticTractionSeparation`.
 **/
class ADPureElasticTractionSeparation : public ADCZMComputeLocalTractionTotalBase
{
public:
  static InputParameters validParams();
  ADPureElasticTractionSeparation(const InputParameters & parameters);

protected:
  virtual void computeInterfaceTraction();

  const RankTwoTensor _K;
};
