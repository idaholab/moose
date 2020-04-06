//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BondStatusBasePD.h"
#include "RankTwoTensor.h"

/**
 * Rank two tensor based failure ctriteria to update the bond status
 * for non-ordinary state-based model
 */
class RankTwoBasedFailureCriteriaNOSPD : public BondStatusBasePD
{
public:
  static InputParameters validParams();

  RankTwoBasedFailureCriteriaNOSPD(const InputParameters & parameters);

protected:
  virtual Real computeFailureCriterionValue() override;

  /// MooseEnum used to control which failure criterion to use
  MooseEnum _failure_criterion;

  /// Material property containing the rank two tensor
  const MaterialProperty<RankTwoTensor> * _tensor;
};
