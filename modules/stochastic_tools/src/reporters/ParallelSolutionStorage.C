//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelSolutionStorage.h"
#include "NonlinearSystemBase.h"

registerMooseObject("StochasticToolsApp", ParallelSolutionStorage);

InputParameters
ParallelSolutionStorage::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Something.");
  return params;
}

ParallelSolutionStorage::ParallelSolutionStorage(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _distributed_solutions(
        declareRestartableData<
            std::vector<std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>>>>(
            "distributed_solution")),
    _local_sample_ids(
        declareRestartableData<std::vector<std::vector<unsigned int>>>("local_sample_ids")),
    _variable_names(declareRestartableData<std::vector<VariableName>>("variable_names"))
{
}

void
ParallelSolutionStorage::initialSetup()
{
  _distributed_solutions.clear();
  _local_sample_ids.clear();
}

void
ParallelSolutionStorage::addEntry(const VariableName & vname,
                                  unsigned int global_i,
                                  std::unique_ptr<DenseVector<Real>> solution)
{
  auto var_it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  if (var_it == _variable_names.end())
  {
    _distributed_solutions.push_back(
        std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>>());
    _variable_names.push_back(vname);
    _local_sample_ids.push_back(std::vector<unsigned int>());
    var_it = --_variable_names.end();
  }
  unsigned int var_i = std::distance(_variable_names.begin(), var_it);

  auto sample_it =
      std::find(_local_sample_ids[var_i].begin(), _local_sample_ids[var_i].end(), global_i);
  if (sample_it == _local_sample_ids[var_i].end())
  {
    _distributed_solutions[var_i].push_back(std::vector<std::unique_ptr<DenseVector<Real>>>());
    _local_sample_ids[var_i].push_back(global_i);
    sample_it = --_local_sample_ids[var_i].end();
  }
  unsigned int sample_i = std::distance(_local_sample_ids[var_i].begin(), sample_it);

  _distributed_solutions[var_i][sample_i].push_back(std::move(solution));
}

unsigned int
ParallelSolutionStorage::totalNumberOfStoredSolutions(const VariableName & vname)
{
  auto var_it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  if (var_it == _variable_names.end())
  {
    mooseError("Variable ",
               vname,
               " does not exist in ParallelSolutionStorage! Available variables are ",
               Moose::stringify(_variable_names));
  }

  unsigned int count = 0;
  for (const auto & sample : _distributed_solutions[std::distance(_variable_names.begin(), var_it)])
    count += sample.size();

  return count;
}

void
ParallelSolutionStorage::printEntries()
{
  std::ostringstream mystream;
  mystream << "Processor " << processor_id() << std::endl;
  for (unsigned int var_i : index_range(_variable_names))
  {
    mystream << "Variable: " << _variable_names[var_i] << std::endl;
    for (unsigned int sample_i : index_range(_local_sample_ids[var_i]))
    {
      mystream << "Sample: " << _local_sample_ids[var_i][sample_i] << std::endl;
      for (const auto & solution : _distributed_solutions[var_i][sample_i])
        mystream << Moose::stringify(solution->get_values()) << std::endl;
    }
  }
  std::cerr << mystream.str() << std::endl;
}
