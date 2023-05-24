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
  params.addRequiredParam<std::vector<VariableName>>(
      "variables",
      "The names of the variables whose serialized solution this object is supposed to receive.");

  // Making this required to make sure nobody prints full solution vectors on accident
  params.makeParamRequired<std::vector<OutputName>>("outputs");
  return params;
}

ParallelSolutionStorage::ParallelSolutionStorage(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _distributed_solutions(
        declareValueByName<
            std::map<VariableName,
                     std::unordered_map<unsigned int, std::vector<DenseVector<Real>>>>>(
            "parallel_solution_storage", REPORTER_MODE_DISTRIBUTED)),
    _variable_names(getParam<std::vector<VariableName>>("variables"))
{
  for (const auto & vname : _variable_names)
    _distributed_solutions.emplace(
        vname, std::unordered_map<unsigned int, std::vector<DenseVector<Real>>>());
}

void
ParallelSolutionStorage::initialSetup()
{
  for (const auto & vname : _variable_names)
    _distributed_solutions[vname].clear();
}

void
ParallelSolutionStorage::addEntry(const VariableName & vname,
                                  unsigned int global_i,
                                  const DenseVector<Real> & solution)
{
  mooseAssert(std::find(_variable_names.begin(), _variable_names.end(), vname) !=
                  _variable_names.end(),
              "We are trying to add a variable that we cannot receive!");

  auto sample_insert_pair =
      _distributed_solutions[vname].emplace(global_i, std::vector<DenseVector<Real>>());

  sample_insert_pair.first->second.push_back(std::move(solution));
}

unsigned int
ParallelSolutionStorage::totalNumberOfStoredSolutions(const VariableName & vname) const
{
  const auto & samples = libmesh_map_find(_distributed_solutions, vname);

  return std::accumulate(
      samples.begin(),
      samples.end(),
      0,
      [](unsigned int count, const std::pair<unsigned int, std::vector<DenseVector<Real>>> & sample)
      { return std::move(count) + sample.second.size(); });
}

bool
ParallelSolutionStorage::hasGlobalSample(unsigned int global_sample_i,
                                         const VariableName & variable) const
{
  if (_distributed_solutions.find(variable) == _distributed_solutions.end())
    return false;

  auto & variable_storage = libmesh_map_find(_distributed_solutions, variable);

  return (variable_storage.find(global_sample_i) != variable_storage.end());
}

const std::vector<DenseVector<Real>> &
ParallelSolutionStorage::getGlobalSample(unsigned int global_sample_i,
                                         const VariableName & variable) const
{
  mooseAssert(_distributed_solutions.find(variable) != _distributed_solutions.end(),
              "We don't have the requested variable!");
  const auto & variable_storage = libmesh_map_find(_distributed_solutions, variable);
  mooseAssert(variable_storage.find(global_sample_i) != variable_storage.end(),
              "We don't have the requested global sample index! ");

  return libmesh_map_find(variable_storage, global_sample_i);
}

std::unordered_map<unsigned int, std::vector<DenseVector<Real>>> &
ParallelSolutionStorage::getStorage(const VariableName & variable)
{
  if (_distributed_solutions.find(variable) == _distributed_solutions.end())
    mooseError(
        "We are trying to access container for variable '", variable, "' but we don't have it!");

  return libmesh_map_find(_distributed_solutions, variable);
}

const std::unordered_map<unsigned int, std::vector<DenseVector<Real>>> &
ParallelSolutionStorage::getStorage(const VariableName & variable) const
{
  if (_distributed_solutions.find(variable) == _distributed_solutions.end())
    mooseError(
        "We are trying to access container for variable '", variable, "' but we don't have it!");

  return libmesh_map_find(_distributed_solutions, variable);
}

void
to_json(
    nlohmann::json & json,
    const std::map<VariableName, std::unordered_map<unsigned int, std::vector<DenseVector<Real>>>> &
        solution_storage)
{
  for (const auto & vname_pair : solution_storage)
  {
    auto & variable_storage = json[vname_pair.first];
    for (const auto & sample_pair : vname_pair.second)
    {
      auto & sample_storage = variable_storage[std::to_string(sample_pair.first)];
      sample_storage = std::vector<std::vector<Real>>();
      for (const auto & sample : sample_pair.second)
      {
        sample_storage.push_back(sample.get_values());
      }
    }
  }
}
