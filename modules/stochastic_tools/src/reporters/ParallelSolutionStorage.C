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
            std::map<VariableName,
                     std::map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>>>(
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
  auto variable_insert_pair = _distributed_solutions.emplace(
      vname, std::map<unsigned int, std::vector<std::unique_ptr<DenseVector<Real>>>>());

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

void
ParallelSolutionStorage::printEntries()
{
  std::ostringstream mystream;
  mystream << "Processor " << processor_id() << std::endl;
  for (const auto & var_it : _distributed_solutions)
  {
    mystream << "Variable: " << var_it.first << std::endl;
    for (const auto & sample_it : var_it.second)
    {
      mystream << "Sample: " << sample_it.first << std::endl;
      for (const auto & solution : sample_it.second)
        mystream << Moose::stringify(solution->get_values()) << std::endl;
    }
  }
  std::cerr << mystream.str() << std::endl;
}
