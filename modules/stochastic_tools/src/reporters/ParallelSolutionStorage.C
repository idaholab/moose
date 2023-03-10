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
    _distributed_solutions(declareRestartableData<
                           std::map<VariableName, std::vector<std::unique_ptr<DenseVector<Real>>>>>(
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
                                  const std::vector<std::unique_ptr<DenseVector<Real>>> & solutions)
{
  // std::vector<std::unique_ptr<std::vector<Real>>> & variable_container =
  //     _distributed_solutions[vname];

  // for (const auto & solution : solutions)
  // {
  //   std::unique_ptr<std::vector<Real>> copied_solution =
  //       std::make_unique<std::vector<Real>>(*solution);
  //   variable_container.push_back(std::move(copied_solution));
  // }

  // std::ostringstream mystream;
  // mystream << "Processor " << processor_id() << std::endl;
  // mystream << "Adding to: " << vname << std::endl;
  // for (const auto & solution : variable_container)
  //   mystream << Moose::stringify(*solution) << std::endl;
  // std::cerr << mystream.str() << std::endl;
}
