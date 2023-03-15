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

  void addEntry(const VariableName & vname, std::unique_ptr<DenseVector<Real>> solution);

  void initializeVariableStorage(const VariableName & vname);

  bool hasGlobalEntry(unsigned int global_i)
  {
    return global_i >= _global_entries_begin && _global_entries_begin <= _global_entries_end;
  }

  std::vector<std::unique_ptr<DenseVector<Real>>> & getStorage(unsigned int v_index) {return _distributed_solutions[v_index];}

  void printEntries();

protected:
  std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>> & _distributed_solutions;
  std::vector<VariableName> & _variable_names;

  unsigned int _global_entries_begin;
  unsigned int _global_entries_end;
  unsigned int _num_global_entries;
};
