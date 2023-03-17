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

  std::vector<std::unique_ptr<DenseVector<Real>>> & getStorage(unsigned int sample_i,
                                                               unsigned int variable_i)
  {
    return _distributed_solutions[variable_i][sample_i];
  }

  std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>> & getStorage(unsigned int variable_i)
  {
    return _distributed_solutions[variable_i];
  }

  std::vector<std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>>> & getStorage()
  {
    return _distributed_solutions;
  }

  void updateTimeStepNumbers();

  unsigned int totalNumberOfStoredSolutions(const VariableName & vname);

  void printEntries();

protected:
  std::vector<std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>>> &
      _distributed_solutions;
  std::vector<std::vector<unsigned int>> & _local_sample_ids;
  std::vector<VariableName> & _variable_names;
};
