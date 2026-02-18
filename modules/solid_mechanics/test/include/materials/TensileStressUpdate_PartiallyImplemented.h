//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensileStressUpdate.h"

/**
 * Just like TensileStressUpdate, but uses the base class,
 * MultiParameterPlasticityStressUpdate::consistentTangentOperatorV
 * instead of the optimised version found in TensileStressUpdate.
 * This is just to check that the base class implementation is
 * correct, if a little compute-heavy.
 */
class TensileStressUpdate_PartiallyImplemented : public TensileStressUpdate
{
public:
  static InputParameters validParams();

  TensileStressUpdate_PartiallyImplemented(const InputParameters & parameters);

protected:
  virtual void consistentTangentOperatorV(const RankTwoTensor & stress_trial,
                                          const std::vector<Real> & trial_stress_params,
                                          const RankTwoTensor & stress,
                                          const std::vector<Real> & stress_params,
                                          Real gaE,
                                          const yieldAndFlow & smoothed_q,
                                          const RankFourTensor & Eijkl,
                                          bool compute_full_tangent_operator,
                                          const std::vector<std::vector<Real>> & dvar_dtrial,
                                          RankFourTensor & cto) override;
};
