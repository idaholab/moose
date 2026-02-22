//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CappedWeakPlaneStressUpdate.h"

/**
 * Just like CappedWeakPlaneStressUpdate, but uses the base class,
 * TwoParameterPlasticityStressUpdate::consistentTangentOperator
 * instead of the optimised version found in CappedWeakPlaneStressUpdate.
 * This is just to check that the base class implementation is
 * correct, if a little compute-heavy.
 */
class CappedWeakPlaneStressUpdate_PartiallyImplemented : public CappedWeakPlaneStressUpdate
{
public:
  static InputParameters validParams();

  CappedWeakPlaneStressUpdate_PartiallyImplemented(const InputParameters & parameters);

protected:
  virtual void consistentTangentOperator(const RankTwoTensor & stress_trial,
                                         Real p_trial,
                                         Real q_trial,
                                         const RankTwoTensor & stress,
                                         Real p,
                                         Real q,
                                         Real gaE,
                                         const yieldAndFlow & smoothed_q,
                                         const RankFourTensor & Eijkl,
                                         bool compute_full_tangent_operator,
                                         RankFourTensor & cto) const override;
};
