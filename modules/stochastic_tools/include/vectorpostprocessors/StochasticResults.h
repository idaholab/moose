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
#include "StochasticResultsAction.h"

/**
 * Storage helper for managing data being assigned to this VPP by a Transfer object.
 */
struct StochasticResultsData
{
  StochasticResultsData(const VectorPostprocessorName & name, VectorPostprocessorValue *);
  VectorPostprocessorName name;
  VectorPostprocessorValue * vector;
  VectorPostprocessorValue current;
};

/**
 * A tool for output Sampler data.
 */
class StochasticResults : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  StochasticResults(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override {}

  // This method is used by the Transfers for populating data within the vectors for this VPP; this
  // is required to allow for the "contains_complete_history = true" to operate correctly with the
  // different modes of parallel operation, without the Transfer objects needing knowledge of the
  // modes.
  //
  // In practice, the Transfer objects are responsible for populating the vector that contains the
  // current data from sub-application for this local process. This object then handles the
  // communication and updating of the actual VPP data being mindfull of the
  // "contains_complete_history". An r-value reference is used by this object to take ownership of
  // the data to avoid unnecessary copying, because the supplied data can be very large.
  //
  // For an example use, please refer to SamplerPostprocessorTransfer.
  //
  // @param vector_name: name of the vector to populated, should be the name of the sampler.
  // @paran current: current local VPP data from sub-applications (see SamplerPostprocessorTranfer)
  void setCurrentLocalVectorPostprocessorValue(const std::string & vector_name,
                                               const VectorPostprocessorValue && current);

protected:
  /// Storage for declared vectors
  std::vector<StochasticResultsData> _sample_vectors;

  /**
   * Create a VPP vector for results data for a given Sampler, see StochasticResultsAction for
   * more details to why this is necessary.
   */
  void initVector(const std::string & vector_name);
  friend void StochasticResultsAction::act();
};
