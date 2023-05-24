//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SerializedSolutionTransfer.h"
#include "NonlinearSystemBase.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", SerializedSolutionTransfer);

InputParameters
SerializedSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription(
      "Serializes and transfers solution vectors for given variables from sub-applications.");
  params.addRequiredParam<std::string>("parallel_storage",
                                       "The name of the parallel storage reporter.");
  params.addRequiredParam<std::string>("solution_container",
                                       "The name of the solution container on the subapp.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variables",
      "The names of the variables which should be serialized and transferred to this application.");
  params.addParam<bool>("serialize_on_root",
                        false,
                        "If we want to gather the solution fields only on the root processors of "
                        "the subapps before transfering to the main app.");
  return params;
}

SerializedSolutionTransfer::SerializedSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _variable_names(getParam<std::vector<VariableName>>("variables")),
    _serialize_on_root(getParam<bool>("serialize_on_root"))
{
  if (hasToMultiApp())
    paramError("to_multi_app", "To and between multiapp directions are not implemented");
}

void
SerializedSolutionTransfer::initialSetup()
{
  // Check if we have the storage space to receive the serialized solution fields
  _parallel_storage = &_fe_problem.getUserObject<ParallelSolutionStorage>(
      getParam<std::string>("parallel_storage"));
}

void
SerializedSolutionTransfer::initializeInNormalMode()
{
  _solution_container.clear();
  const auto n = getFromMultiApp()->numGlobalApps();
  const auto & serialized_solution_reporter = getParam<std::string>("solution_container");

  for (MooseIndex(n) i = 0; i < n; i++)
    if (getFromMultiApp()->hasLocalApp(i))
    {
      FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(i);
      _solution_container.push_back(
          &app_problem.getUserObject<SolutionContainer>(serialized_solution_reporter));
    }
}

void
SerializedSolutionTransfer::initializeInBatchMode()
{
  // First we fetch the solution containers from the subapps. This function is used
  // in batch mode only so we will have one solution container on each rank
  _solution_container.clear();

  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(_app_index);

  _solution_container.push_back(
      &app_problem.getUserObject<SolutionContainer>(getParam<std::string>("solution_container")));
}

void
SerializedSolutionTransfer::execute()
{
  initializeInNormalMode();

  const auto n = getFromMultiApp()->numGlobalApps();

  for (MooseIndex(n) i = 0; i < n; i++)
  {
    if (getFromMultiApp()->hasLocalApp(i))
    {
      FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(i);
      NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();

      // Converting the local indexing to global sample indices
      const unsigned int local_i = i - _sampler_ptr->getLocalRowBegin();

      // Here we have to branch out based on if only the root processors
      // need to participate in the transfer or if we would like to distribute the
      // data among every processor of the subapplication
      if (_serialize_on_root)
        transferToSubAppRoot(nl, *_solution_container[local_i], i);
      else
        transferInParallel(nl, *_solution_container[local_i], i);
    }
  }
}

void
SerializedSolutionTransfer::executeFromMultiapp()
{
  initializeInBatchMode();

  if (getFromMultiApp()->hasLocalApp(_app_index))
  {
    FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(_app_index);
    NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();

    // Here we have to branch out based on if only the root processors
    // need to participate in the transfer or if we would like to distribute the
    // data among every processor of the subapplication
    if (_serialize_on_root)
      transferToSubAppRoot(nl, *_solution_container[0], _global_index);
    else
      transferInParallel(nl, *_solution_container[0], _global_index);
  }
}

void
SerializedSolutionTransfer::transferInParallel(NonlinearSystemBase & app_nl_system,
                                               SolutionContainer & solution_container,
                                               const dof_id_type global_i)
{
  // We need to go through this communicator because the multiapp's
  // communicator is not necessarily the communicator of the underlying MooseObject.

  const auto & comm = app_nl_system.comm();
  dof_id_type num_entries = _sampler_ptr->getNumberOfLocalRows();
  comm.sum(num_entries);

  // We shall distribute the samples on the given application between its processors.
  // Only using a linear partitioning here for the sake of simplicity.
  dof_id_type new_local_entries_begin;
  dof_id_type new_local_entries_end;
  dof_id_type num_new_local_entries;

  MooseUtils::linearPartitionItems(num_entries,
                                   comm.size(),
                                   comm.rank(),
                                   num_new_local_entries,
                                   new_local_entries_begin,
                                   new_local_entries_end);

  unsigned int local_app_index = global_i - _sampler_ptr->getLocalRowBegin();

  // Looping over the variables to extract the corresponding solution values
  // and copy them into the container.
  for (unsigned int var_i = 0; var_i < _variable_names.size(); ++var_i)
  {
    // Getting the corresponding DoF indices for the variable.
    app_nl_system.setVariableGlobalDoFs(_variable_names[var_i]);

    for (unsigned int solution_i = 0; solution_i < solution_container.getContainer().size();
         ++solution_i)
    {
      DenseVector<Real> serialized_solution;

      // Localize the solution and add it to the local container on the rank
      // which is supposed to own it

      solution_container.getSolution(solution_i)
          ->localize(serialized_solution.get_values(),
                     (local_app_index >= new_local_entries_begin &&
                      local_app_index < new_local_entries_end)
                         ? app_nl_system.getVariableGlobalDoFs()
                         : std::vector<dof_id_type>());

      if (local_app_index >= new_local_entries_begin && local_app_index < new_local_entries_end)
        _parallel_storage->addEntry(_variable_names[var_i], global_i, serialized_solution);
    }
  }
}

void
SerializedSolutionTransfer::transferToSubAppRoot(NonlinearSystemBase & app_nl_system,
                                                 SolutionContainer & solution_container,
                                                 const dof_id_type global_i)
{
  // Looping over the variables to extract the corresponding solution values
  for (unsigned int var_i = 0; var_i < _variable_names.size(); ++var_i)
  {
    // Getting the corresponding DoF indices for the variable.
    app_nl_system.setVariableGlobalDoFs(_variable_names[var_i]);

    for (unsigned int solution_i = 0; solution_i < solution_container.getContainer().size();
         ++solution_i)
    {
      DenseVector<Real> serialized_solution;

      // In this case we always serialize on the root processor of the application.
      solution_container.getSolution(solution_i)
          ->localize(serialized_solution.get_values(),
                     getFromMultiApp()->isRootProcessor() ? app_nl_system.getVariableGlobalDoFs()
                                                          : std::vector<dof_id_type>());

      if (getFromMultiApp()->isRootProcessor())
        _parallel_storage->addEntry(_variable_names[var_i], global_i, serialized_solution);
    }
  }
}
