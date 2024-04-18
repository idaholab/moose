//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include "Sampler.h"
#include <torch/torch.h>
#include "LibtorchANNSurrogate.h"
#include "SurrogateModel.h"
#include "SurrogateModelInterface.h"
#include "Standardizer.h"
#include "GaussianProcess.h"
#include "TransientInterface.h"

/**
 * Fast Bayesian inference with the GPry algorithm by El Gammal et al. 2023: sampler step
 */
class BayesianGPrySampler : public Sampler,
                            public SurrogateModelInterface,
                            public TransientInterface
{
public:
  static InputParameters validParams();

  BayesianGPrySampler(const InputParameters & parameters);

  /**
   * Returns true if the adaptive sampling is completed
   */
  virtual bool isAdaptiveSamplingCompleted() const override { return _is_sampling_completed; }

//   using SurrogateModel::evaluate;
//   virtual Real evaluate(const std::vector<Real> & x) const override;

protected:
  /// Gather all the samples
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// True if the sampling is completed
  bool _is_sampling_completed = false;

  /// Get the libtorch classifer neural network
  LibtorchANNSurrogate & _nn;

  /// Get the Gaussian process surrogate of log-posterior
  const SurrogateModel & _gp;

private:
  /// Number of samples requested
  const int & _num_iterations;

  /// Maximum number of subApp calls in each iteration
  const unsigned int & _num_samples;

  /// Number of samples to propose in each iteration (not all are sent for subApp evals)
  const unsigned int & _num_tries;

  /// Ensure that the sampler proceeds in a sequential fashion
  int _check_step;

  /// Storage for all the proposed samples
  std::vector<std::vector<Real>> _inputs_all;

  /// Storage for the selected samples sent to subApp evals
  std::vector<std::vector<Real>> _inputs_subapp;

  /// Store the GP mean
  std::vector<Real> _gp_mean;

  /// Store the GP standard deviation
  std::vector<Real> _gp_std;

  /// Store the NN classifer outputs
  std::vector<Real> _nn_outputs;
};

#endif
