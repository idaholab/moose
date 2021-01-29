//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to perform Monte Carlo Sampling
 *
 * WARNING! This Sampler manages the generators advancement for parallel operation manually. All the
 *          calls to get random values from the generators occur in sampleSetUp. At the end of
 *          sampleSetUp all the generators should be in the final state.
 */
class LatinHypercubeSampler : public Sampler
{
public:
  static InputParameters validParams();

  LatinHypercubeSampler(const InputParameters & parameters);

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

private:
  // Probability values for each distribution
  std::vector<std::vector<Real>> _probabilities;

  // toggle for local/global data access in computeSample
  bool _is_local;
};
