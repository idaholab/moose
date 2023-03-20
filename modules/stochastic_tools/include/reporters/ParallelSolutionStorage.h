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

class ParallelSolutionStorage : public GeneralReporter
{
public:
  static InputParameters validParams();
  ParallelSolutionStorage(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override{};
  virtual void execute() override{};
  virtual void finalize() override {}

  void addEntry(const VariableName & vname,
                unsigned int global_i,
                std::unique_ptr<DenseVector<Real>> solution);

  std::map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>> &
  getStorage(const VariableName & variable)
  {
    mooseAssert(_distributed_solutions.find(variable) != _distributed_solutions.end(),
                "We don't have the requested variable!");

    return libmesh_map_find(_distributed_solutions, variable);
  }

  std::map<VariableName, std::map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>> &
  getStorage()
  {
    return _distributed_solutions;
  }

  bool hasGlobalSample(unsigned int global_sample_i, const VariableName & variable)
  {
    if (_distributed_solutions.find(variable) == _distributed_solutions.end())
      return false;

    auto & variable_storage = libmesh_map_find(_distributed_solutions, variable);

    return (variable_storage.find(global_sample_i) != variable_storage.end());
  }

  const std::vector<std::unique_ptr<DenseVector<Real>>> &
  getGlobalSample(unsigned int global_sample_i, const VariableName & variable)
  {
    mooseAssert(_distributed_solutions.find(variable) != _distributed_solutions.end(),
                "We don't have the requested variable!");
    const auto & variable_storage = libmesh_map_find(_distributed_solutions, variable);
    mooseAssert(variable_storage.find(global_sample_i) != variable_storage.end(),
                "We don't have the requested global sample index! ");

    return libmesh_map_find(variable_storage, global_sample_i);
  }

  void updateTimeStepNumbers();

  unsigned int totalNumberOfStoredSolutions(const VariableName & vname);

  void printEntries();

protected:
  std::map<VariableName, std::map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>> &
      _distributed_solutions;
};
