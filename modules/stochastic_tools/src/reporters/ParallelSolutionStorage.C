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
        declareRestartableData<std::vector<std::vector<std::unique_ptr<DenseVector<Real>>>>>(
            "distributed_solution")),
    _variable_names(declareRestartableData<std::vector<VariableName>>("variable_names"))
{
}

void
ParallelSolutionStorage::initialSetup()
{
  _distributed_solutions.clear();
}

void
ParallelSolutionStorage::initializeVariableStorage(const VariableName & vname)
{
  if (std::find(_variable_names.begin(), _variable_names.end(), vname) == _variable_names.end())
  {
    _variable_names.push_back(vname);
    _distributed_solutions.push_back(std::vector<std::unique_ptr<DenseVector<Real>>>());
  }
}

void
ParallelSolutionStorage::addEntry(const VariableName & vname,
                                  std::unique_ptr<DenseVector<Real>> solution)
{
  const auto & it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  if (it == _variable_names.end())
    mooseError("The sorage has not been initialized yet for ",
               vname,
               " call initializeVariableStorage first!");

  _distributed_solutions[it - _variable_names.begin()].push_back(std::move(solution));
}

void
ParallelSolutionStorage::printEntries()
{
  std::ostringstream mystream;
  mystream << "Processor " << processor_id() << std::endl;
  for (unsigned int vari : index_range(_variable_names))
  {
    mystream << "Variable: " << _variable_names[vari] << std::endl;
    for (const auto & solution : _distributed_solutions[vari])
      mystream << Moose::stringify(solution->get_values()) << std::endl;
  }
  std::cerr << mystream.str() << std::endl;
}
