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
#include "VectorPacker.h"
#include "timpi/communicator.h"

registerMooseObject("StochasticToolsApp", SerializedSolutionTransfer);

InputParameters
SerializedSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Noice.");
  params.addRequiredParam<std::string>("parallel_storage_name", "Something here.");
  params.addRequiredParam<std::string>("serialized_solution_reporter", "Something here.");
  params.addRequiredParam<std::vector<VariableName>>("variables", "Something.");
  params.addParam<bool>(
      "serialize_on_root",
      false,
      "If we only want to gather the solution fields on the root procesors of the subapps.");
  return params;
}

SerializedSolutionTransfer::SerializedSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _variable_names(getParam<std::vector<VariableName>>("variables")),
    _serialized_solution_reporter(getParam<std::string>("serialized_solution_reporter")),
    _serialize_on_root(getParam<bool>("serialize_on_root"))
{
}

void
SerializedSolutionTransfer::initialSetup()
{
  std::string parallel_storage_name = getParam<std::string>("parallel_storage_name");

  std::vector<UserObject *> reporters;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "parallel_storage_name", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("parallel_storage_name",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!_parallel_storage)
    paramError("parallel_storage_name",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");
}

void
SerializedSolutionTransfer::execute()
{
}

void
SerializedSolutionTransfer::initializeFromMultiapp()
{
}

void
SerializedSolutionTransfer::executeFromMultiapp()
{
  // Getting the reference to the solution vector in the subapp.
  FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(_app_index);

  std::vector<UserObject *> reporters;
  app_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(_serialized_solution_reporter)
      .queryInto(reporters);

  if (reporters.empty())
    paramError("serialized_solution_reporter",
               "Unable to find reporter with name '",
               _serialized_solution_reporter,
               "'");
  else if (reporters.size() > 1)
    paramError("serialized_solution_reporter",
               "We found more than one reporter with the name '",
               _serialized_solution_reporter,
               "'");

  SolutionContainer * solution_container = dynamic_cast<SolutionContainer *>(reporters[0]);

  if (!solution_container)
    paramError("serialized_solution_reporter",
               "The parallel storage reporter is not of type '",
               _serialized_solution_reporter,
               "'");

  if (getFromMultiApp()->hasLocalApp(_app_index))
  {
    FEProblemBase & app_problem = getFromMultiApp()->appProblemBase(_app_index);
    NonlinearSystemBase & nl = app_problem.getNonlinearSystemBase();

    if (_serialize_on_root)
      transferToRoot(nl, *solution_container);
    else
      transferInParallel(nl, *solution_container);
  }

  _parallel_storage->printEntries();
}

void
SerializedSolutionTransfer::transferInParallel(NonlinearSystemBase & app_nl_system,
                                               SolutionContainer & solution_container)
{
  dof_id_type new_local_entries_begin;
  dof_id_type new_local_entries_end;
  dof_id_type num_new_local_entries;

  int local_size;
  int local_rank;
  const MPI_Comm & app_comm = getFromMultiApp()->comm();
  MPI_Comm_size(app_comm, &local_size);
  MPI_Comm_rank(app_comm, &local_rank);

  dof_id_type num_entries = _sampler_ptr->getNumberOfLocalRows();
  dof_id_type num_app_entries = num_entries;

  if (local_size > 1)
    MPI_Allreduce(&num_entries,
                  &num_app_entries,
                  1,
                  TIMPI::StandardType<dof_id_type>(&num_entries),
                  TIMPI::OpFunction<dof_id_type>::sum(),
                  app_comm);

  MooseUtils::linearPartitionItems(num_app_entries,
                                   local_size,
                                   local_rank,
                                   num_new_local_entries,
                                   new_local_entries_begin,
                                   new_local_entries_end);

  unsigned int local_app_index = _global_index - _sampler_ptr->getLocalRowBegin();

  // Looping over the variables to extract the corresponding solution values
  // and copy them into the container of the trainer.
  for (unsigned int var_i = 0; var_i < _variable_names.size(); ++var_i)
  {
    // Getting the corresponding DoF indices for the variable.
    app_nl_system.setVariableGlobalDoFs(_variable_names[var_i]);

    for (unsigned int solution_i = 0; solution_i < solution_container.getContainer().size();
         ++solution_i)
    {
      std::unique_ptr<DenseVector<Real>> serialized_solution =
          std::make_unique<DenseVector<Real>>();
      solution_container.getSolution(solution_i)
          ->localize(serialized_solution->get_values(),
                     (local_app_index >= new_local_entries_begin &&
                      local_app_index < new_local_entries_end)
                         ? app_nl_system.getVariableGlobalDoFs()
                         : std::vector<dof_id_type>());

      if (local_app_index >= new_local_entries_begin && local_app_index < new_local_entries_end)
        _parallel_storage->addEntry(
            _variable_names[var_i], _global_index, std::move(serialized_solution));
    }
  }
}

void
SerializedSolutionTransfer::transferToRoot(NonlinearSystemBase & app_nl_system,
                                           SolutionContainer & solution_container)
{
  // Looping over the variables to extract the corresponding solution values
  // and copy them into the container of the trainer.
  for (unsigned int var_i = 0; var_i < _variable_names.size(); ++var_i)
  {
    // Getting the corresponding DoF indices for the variable.
    app_nl_system.setVariableGlobalDoFs(_variable_names[var_i]);

    for (unsigned int solution_i = 0; solution_i < solution_container.getContainer().size();
         ++solution_i)
    {
      std::unique_ptr<DenseVector<Real>> serialized_solution =
          std::make_unique<DenseVector<Real>>();
      solution_container.getSolution(solution_i)
          ->localize(serialized_solution->get_values(),
                     getFromMultiApp()->isRootProcessor() ? app_nl_system.getVariableGlobalDoFs()
                                                          : std::vector<dof_id_type>());

      if (getFromMultiApp()->isRootProcessor())
        _parallel_storage->addEntry(
            _variable_names[var_i], _global_index, std::move(serialized_solution));
    }
  }
}

void
SerializedSolutionTransfer::finalizeFromMultiapp()
{
}

void
SerializedSolutionTransfer::initializeToMultiapp()
{
}

void
SerializedSolutionTransfer::executeToMultiapp()
{
}

void
SerializedSolutionTransfer::finalizeToMultiapp()
{
}
