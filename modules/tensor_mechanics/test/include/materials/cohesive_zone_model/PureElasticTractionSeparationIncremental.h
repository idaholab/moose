//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeLocalTractionIncrementalBase.h"

/**
 * Implementation of the non-stateful exponential traction separation law
 * proposed by Salehani, Mohsen Khajeh and Irani, Nilgoon 2018
 **/
class PureElasticTractionSeparationIncremental : public CZMComputeLocalTractionIncrementalBase
{
public:
  static InputParameters validParams();
  PureElasticTractionSeparationIncremental(const InputParameters & parameters);

protected:
  /// method computing the total traction and its derivatives
  void computeInterfaceTractionIncrementAndDerivatives() override;

  /// the vector representing the maximum allowed traction in each direction
  const RankTwoTensor _K;
};
