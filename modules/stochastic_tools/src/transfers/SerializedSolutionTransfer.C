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

registerMooseObject("StochasticToolsApp", SerializedSolutionTransfer);

InputParameters
SerializedSolutionTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Noice.");
  params.addRequiredParam<std::string>("parallel_storage_name", "Something here.");
  params.addRequiredParam<std::string>("serialized_solution_reporter", "Something here.");
  params.addRequiredParam<std::vector<VariableName>>("variables", "Something.");
  return params;
}

SerializedSolutionTransfer::SerializedSolutionTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    _variable_names(getParam<std::vector<VariableName>>("variables")),
    _serialized_solution_reporter(getParam<std::string>("serialized_solution_reporter")),
    _num_global_entries(0),
    _num_local_entries(0)
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

  for (const auto & vname : _variable_names)
    _parallel_storage->initializeVariableStorage(vname);

  unsigned int _num_true_global_apps = _app_index + 1;
  comm().max(_num_true_global_apps);

  _root_processors.clear();
  _root_processors.resize(_num_true_global_apps);
  if (getFromMultiApp()->isRootProcessor())
  {
    _root_processors[_app_index] = processor_id();
  }
  comm().sum(_root_processors);
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
  unsigned int num_incoming_local_entries = 0;
  unsigned int num_incoming_global_entries = 0;

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

  unsigned int true_num_local_apps = _app_index;
  comm().max(true_num_local_apps);

  num_incoming_local_entries = solution_container->getContainer().size();
  std::vector<unsigned int> local_apps(true_num_local_apps + 1, 0);

  if (getFromMultiApp()->isRootProcessor())
  {
    local_apps[_app_index] = num_incoming_local_entries;
  }
  comm().sum(local_apps);

  std::vector<unsigned int> local_already_in_container(n_processors(), 0);
  local_already_in_container[processor_id()] = _parallel_storage->getStorage(0).size();
  comm().sum(local_already_in_container);

  std::vector<unsigned int> app_to_processor(n_processors(), 0);
  app_to_processor[processor_id()] = _app_index;
  comm().sum(app_to_processor);

  std::vector<std::pair<unsigned int, unsigned int>> incoming_ranges;
  std::vector<std::pair<unsigned int, unsigned int>> outgoing_ranges;

  createDataPartitioning(local_apps, local_already_in_container, incoming_ranges, outgoing_ranges);

  std::map<unsigned int, processor_id_type> transfer_map;
  fillTransferMap(incoming_ranges, outgoing_ranges, app_to_processor, transfer_map);

  std::unordered_map<processor_id_type,
                     std::vector<std::pair<unsigned int, std::unique_ptr<DenseVector<Real>>>>>
      send_map;

  NonlinearSystemBase & nl = getFromMultiApp()->appProblemBase(_app_index).getNonlinearSystemBase();

  for (const auto & send_entry : transfer_map)
  {
    for (const auto & variable_index : index_range(_variable_names))
    {
      nl.setVariableGlobalDoFs(_variable_names[variable_index]);
      std::unique_ptr<DenseVector<Real>> serialized_solution =
          std::make_unique<DenseVector<Real>>();
      solution_container->getSolution(send_entry.first)
          ->localize(serialized_solution->get_values(),
                     getFromMultiApp()->isRootProcessor() ? nl.getVariableGlobalDoFs()
                                                          : std::vector<dof_id_type>());

      if (getFromMultiApp()->isRootProcessor())
      {
        send_map[send_entry.second].push_back(
            std::make_pair(variable_index, std::move(serialized_solution)));
      }
    }
  }

  for (unsigned int global_i = incoming_ranges[_app_index].first;
       global_i < incoming_ranges[_app_index].second;
       ++global_i)
  {
    for (auto proc_i : make_range(n_processors()))
    {
      if (outgoing_ranges[proc_i].first <= global_i && outgoing_ranges[proc_i].second > global_i &&
          app_to_processor[proc_i] == app_to_processor[processor_id()])
      {

        for (const auto & variable_index : index_range(_variable_names))
        {
          nl.setVariableGlobalDoFs(_variable_names[variable_index]);
          std::unique_ptr<DenseVector<Real>> serialized_solution =
              std::make_unique<DenseVector<Real>>();
          solution_container->getSolution(global_i - incoming_ranges[_app_index].first)
              ->localize(serialized_solution->get_values(),
                         proc_i == processor_id() ? nl.getVariableGlobalDoFs()
                                                  : std::vector<dof_id_type>());
          if (proc_i == processor_id())
          {
            _parallel_storage->addEntry(_variable_names[variable_index],
                                        std::move(serialized_solution));
          }
        }
      }
    }
  }

  auto functor =
      [this](processor_id_type pid,
             std::vector<std::pair<unsigned int, std::unique_ptr<DenseVector<Real>>>> & vectors)
  {
    for (auto & pair : vectors)
    {
      const auto var_i = pair.first;
      auto & vector = pair.second;

      _parallel_storage->addEntry(_variable_names[pair.first], std::move(vector));
    }
  };

  Parallel::push_parallel_packed_range(_communicator, send_map, (void *)nullptr, functor);

  _parallel_storage->printEntries();
}

void
SerializedSolutionTransfer::createDataPartitioning(
    std::vector<unsigned int> & new_snapshots_per_app,
    std::vector<unsigned int> & local_already_in_container,
    std::vector<std::pair<unsigned int, unsigned int>> & incoming_begin_end,
    std::vector<std::pair<unsigned int, unsigned int>> & outgoing_begin_end)
{
  incoming_begin_end.clear();
  outgoing_begin_end.clear();

  unsigned int app_begin = 0;
  for (auto app_i : index_range(new_snapshots_per_app))
  {
    incoming_begin_end.push_back(
        std::make_pair(app_begin, app_begin + new_snapshots_per_app[app_i]));
    app_begin = app_begin + new_snapshots_per_app[app_i];
  }

  unsigned int global_already_in_container =
      std::reduce(local_already_in_container.begin(), local_already_in_container.end());

  unsigned int num_new_snapshots =
      std::reduce(new_snapshots_per_app.begin(), new_snapshots_per_app.end());

  unsigned int incoming_plus_existing = global_already_in_container + num_new_snapshots;

  dof_id_type new_local_entries_begin;
  dof_id_type new_local_entries_end;
  dof_id_type num_new_local_entries;

  MooseUtils::linearPartitionItems(incoming_plus_existing,
                                   n_processors(),
                                   processor_id(),
                                   num_new_local_entries,
                                   new_local_entries_begin,
                                   new_local_entries_end);

  // We check the difference between the optimal and the already existing snapshots
  std::vector<unsigned int> need_to_get_this_many(n_processors(), 0);
  need_to_get_this_many[processor_id()] =
      num_new_local_entries - local_already_in_container[processor_id()];
  comm().sum(need_to_get_this_many);

  unsigned int outgoing_begin = 0;
  for (auto proc_i : make_range(n_processors()))
  {
    outgoing_begin_end.push_back(
        std::make_pair(outgoing_begin, outgoing_begin + need_to_get_this_many[proc_i]));
    outgoing_begin = outgoing_begin + need_to_get_this_many[proc_i];
  }
}

void
SerializedSolutionTransfer::fillTransferMap(
    std::vector<std::pair<unsigned int, unsigned int>> & incoming_range,
    std::vector<std::pair<unsigned int, unsigned int>> & outgoing_range,
    std::vector<unsigned int> & app_to_processor,
    std::map<unsigned int, processor_id_type> & transfer_map)
{
  transfer_map.clear();

  for (auto global_snap_i = incoming_range[_app_index].first;
       global_snap_i < incoming_range[_app_index].second;
       ++global_snap_i)
  {
    for (auto proc_i : make_range(n_processors()))
    {
      if (global_snap_i < outgoing_range[proc_i].second &&
          global_snap_i >= outgoing_range[proc_i].first &&
          app_to_processor[proc_i] != app_to_processor[processor_id()])
      {
        transfer_map[global_snap_i - incoming_range[_app_index].first] = proc_i;
        break;
      }
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
