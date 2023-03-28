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
  params.addClassDescription("Parallel container to store serialized solution fields from "
                             "simulations on sub-applications.");
  return params;
}

ParallelSolutionStorage::ParallelSolutionStorage(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _distributed_solutions(
        declareRestartableData<std::map<
            VariableName,
            std::unordered_map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>>>(
            "distributed_solution"))
{
}

void
ParallelSolutionStorage::initialSetup()
{
  _distributed_solutions.clear();
}

void
ParallelSolutionStorage::addEntry(const VariableName & vname,
                                  unsigned int global_i,
                                  std::unique_ptr<DenseVector<Real>> solution)
{
  // Emplace returns a pair to decide if we inserted a new element or not
  auto variable_insert_pair = _distributed_solutions.emplace(
      vname, std::unordered_map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>());

  auto sample_insert_pair = variable_insert_pair.first->second.emplace(
      global_i, std::vector<std::unique_ptr<DenseVector<Real>>>());

  sample_insert_pair.first->second.push_back(std::move(solution));
}

unsigned int
ParallelSolutionStorage::totalNumberOfStoredSolutions(const VariableName & vname)
{
  const auto & samples = libmesh_map_find(_distributed_solutions, vname);

  unsigned int count = 0;
  for (const auto & sample : samples)
    count += sample.second.size();

  return count;
}

bool
ParallelSolutionStorage::hasGlobalSample(unsigned int global_sample_i,
                                         const VariableName & variable)
{
  if (_distributed_solutions.find(variable) == _distributed_solutions.end())
    return false;

  auto & variable_storage = libmesh_map_find(_distributed_solutions, variable);

  return (variable_storage.find(global_sample_i) != variable_storage.end());
}

const std::vector<std::unique_ptr<DenseVector<Real>>> &
ParallelSolutionStorage::getGlobalSample(unsigned int global_sample_i,
                                         const VariableName & variable)
{
  mooseAssert(_distributed_solutions.find(variable) != _distributed_solutions.end(),
              "We don't have the requested variable!");
  const auto & variable_storage = libmesh_map_find(_distributed_solutions, variable);
  mooseAssert(variable_storage.find(global_sample_i) != variable_storage.end(),
              "We don't have the requested global sample index! ");

  return libmesh_map_find(variable_storage, global_sample_i);
}
