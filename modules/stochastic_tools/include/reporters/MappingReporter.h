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
#include "StochasticReporter.h"
#include "ParallelSolutionStorage.h"
#include "MappingInterface.h"
#include "UserObjectInterface.h"

/**
 * A tool to reduce solution fields to coordinates in the latent space.
 */
class MappingReporter : public StochasticReporter, public MappingInterface
{
public:
  static InputParameters validParams();
  MappingReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  void initialSetup() override;
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Map the available data in a parallel storage into the latent space
  void mapParallelStorageData();

  /// Map the available data in variables into the latent space
  void mapVariableData();

  /// If we already have solution fields stored from previous runs, we can use their
  /// ParallelStorageObject to obtain the corresponding coefficients
  const ParallelSolutionStorage * const _parallel_storage;

  /// We only need the sampler to check which coefficients would go to which processor
  /// in case a ParallelSolutionStorage is used
  Sampler * const _sampler;

  /// The name of the mapping object we would like to use
  const UserObjectName & _mapping_name;
  /// Link to the mapping object, we need this to be a pointer due to the fact that we can only fetch this in initialSetup
  VariableMappingBase * _mapping;

  /// The variables we would like to map
  const std::vector<VariableName> & _variable_names;

  ///@{
  /// Links to the storage spaces (reporters) where we collect the coefficients
  std::vector<std::vector<std::vector<Real>> *> _vector_real_values_parallel_storage;
  std::vector<std::vector<Real> *> _vector_real_values;
  ///@}
};
