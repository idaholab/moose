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

/**
 * A Reporter which stores serialized solution fields for given variables in a distributed fashion.
 * The solutions can be inserted from subapps using the SerializedSolutionTransfer.
 */
class ParallelSolutionStorage : public GeneralReporter
{
public:
  static InputParameters validParams();
  ParallelSolutionStorage(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override {}

  /**
   * Add a new solution entry to the container.
   * @param vname The name of the variable
   * @param global_i The global index of the field
   * @param solution The vector that needs to be added
   */
  void addEntry(const VariableName & vname,
                unsigned int global_i,
                std::unique_ptr<DenseVector<Real>> solution);
  /**
   * Get the stored solution vectors for a given variable
   * @param variable The name of the given variable
   */
  std::unordered_map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>> &
  getStorage(const VariableName & variable)
  {
    mooseAssert(_distributed_solutions.find(variable) != _distributed_solutions.end(),
                "We don't have the requested variable!");

    return libmesh_map_find(_distributed_solutions, variable);
  }

  /// Get the whole solution container
  std::map<VariableName,
           std::unordered_map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>> &
  getStorage()
  {
    return _distributed_solutions;
  }

  /**
   * Determine if we have the solution vector with a given global sample index for a given
   * variable.
   * @param global_sample_i The global sample index of the solution field (fields if it is a t)
   * @param variable The name of the variable whose data is requested
   */
  bool hasGlobalSample(unsigned int global_sample_i, const VariableName & variable);

  /**
   * Get the serialized solution field which is associated with a given
   * global sample index and variable
   * @param global_sample_i The global sampler index
   * @param variable The variable name
   */
  const std::vector<std::unique_ptr<DenseVector<Real>>> &
  getGlobalSample(unsigned int global_sample_i, const VariableName & variable);

  /**
   * Return the number of total stored solutions for a given variable
   * @param vname The name of the variable
   */
  unsigned int totalNumberOfStoredSolutions(const VariableName & vname);

protected:
  /**
   * The container of the solutions. It indexes based on: the variable name > global sample index >
   * timestep index (for time-dependent simulations). This object stores a reference because we
   * would like this to be restartable.
   */
  std::map<VariableName,
           std::unordered_map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>> &
      _distributed_solutions;
};
