//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeLocalTractionTotalBase.h"

/**
 * Implementation of the non-stateful exponential traction separation law
 * proposed by Salehani, Mohsen Khajeh and Irani, Nilgoon 2018
 **/
class SalehaniIrani3DCTraction : public CZMComputeLocalTractionTotalBase
{
public:
  static InputParameters validParams();
  SalehaniIrani3DCTraction(const InputParameters & parameters);

protected:
  void computeInterfaceTractionAndDerivatives() override;

  /// method computing the total traction
  RealVectorValue computeTraction();

  /// method computing the total traction derivatives w.r.t. the interface displacement jump
  RankTwoTensor computeTractionDerivatives();

  /// the displacement jump associated to the maximum traction
  const RealVectorValue _delta_u0;

  /// the vector representing the maximum allowed traction in each direction
  const RealVectorValue _max_allowable_traction;
};
