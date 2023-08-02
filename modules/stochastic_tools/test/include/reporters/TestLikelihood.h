//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "LikelihoodFunctionBase.h"
#include "LikelihoodInterface.h"

/**
 * TestLikelihood will help test new likelihood objects.
 */
class TestLikelihood : public GeneralReporter, public LikelihoodInterface
{
public:
  static InputParameters validParams();
  TestLikelihood(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /// Value of the density or mass function
  std::vector<Real> & _function;

private:
  /// Storage for the likelihood objects to be utilized
  std::vector<const LikelihoodFunctionBase *> _likelihoods;

  /// model prediction values
  const std::vector<Real> & _model_pred;

  /// Transfer the right outputs to the file
  std::vector<Real> & _model_pred_required;

  /// The MCMC sampler
  Sampler & _sampler;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;
};
