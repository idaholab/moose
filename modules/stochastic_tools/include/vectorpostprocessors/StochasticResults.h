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
class StochasticResults : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  static InputParameters validParams();

  StochasticResults(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual finalize() override;
  void virtual execute() override {}

  // This method is used by the Transfers for populating data within the vectors for this VPP; this
  // is required to allow for the "contains_complete_history = true" to operate correctly with the
  // different modes of parallel operation, without the Transfer objects needing knowledge of the
  // modes.
  //
  // In practice, the Trasnfer objects are responsible for populating the vector that contains the
  // current data from sub-application for this local process. This object then handles the
  // communication and updating of the actual VPP data being mindfull of the
  // "contains_complete_history". An r-value reference is used by this object to take ownership of
  // the data to avoid uncesssary copying, because the supplied data can be very large.
  //
  // For an example use, please refer to SamplerPostprocessorTransfer.
  //
  // @param vector_name: name of the vector to populated, should be the name of the sampler.
  // @paran current: current local VPP data from sub-applications (see SamplerPostprocessorTranfer)
  void setCurrentLocalVectorPostprocessorValue(const std::string & vector_name,
                                               const VectorPostprocessorValue && current);

  /**
   * DEPRECATED
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
  std::vector<StochasticResultsData> _sample_vectors;

  /// Parallel operation mode
  const MooseEnum _parallel_type;

  /// The rank data to output if parallel type is distributed
  const processor_id_type _output_distributed_rank;
};
