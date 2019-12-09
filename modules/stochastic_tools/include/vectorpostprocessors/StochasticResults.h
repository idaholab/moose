//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "SamplerInterface.h"

class StochasticResults;

template <>
InputParameters validParams<StochasticResults>();

/**
 * A tool for output Sampler data.
 */
class StochasticResults : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  StochasticResults(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual finalize() override {}
  void virtual execute() override {}

  /**
   * Initialize storage based on the Sampler returned by the SamplerTransientMultiApp or
   * SamplerFullSolveMultiApp.
   * @param sampler The Sampler associated with the MultiApp that this VPP is working with.
   *
   * This an internal method that is called by the SamplerPostprocessorTransfer, it should not
   * be called otherwise.
   */
  void init(Sampler & _sampler);

protected:
  /// Storage for declared vectors
  VectorPostprocessorValue * _sample_vector;

  /// The sampler to extract data
  Sampler * _sampler = nullptr;
};
