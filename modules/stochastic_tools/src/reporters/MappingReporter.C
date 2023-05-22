//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "MappingReporter.h"
#include "NonlinearSystemBase.h"
#include "libmesh/parallel_sync.h"
#include "VectorPacker.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsApp", MappingReporter);

InputParameters
MappingReporter::validParams()
{
  InputParameters params = StochasticReporter::validParams();
  params += SamplerInterface::validParams();
  params += MappingInterface::validParams();
  params.addClassDescription(
      "A reporter which can map full solution fields to a latent space for given variables.");
  params.addRequiredParam<UserObjectName>("mapping", "Name of the mapping object.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variables", "The names of the variables which need to be mapped to the latent space.");
  params.addParam<std::string>("parallel_storage",
                               "The storage space where the snapshots are stored. These snapshots "
                               "are used to build the mapping. If this parameter is not specified, "
                               "the reporter will fetch the variable from the nonlinear system.");
  params.addParam<SamplerName>(
      "sampler",
      "Sampler be able to identify how the samples are distributed among "
      "the processes. Only needed if parallel storage is defined. It is important to have the "
      "same sampler here as the one used to prepare the snapshots in the parallel storage.");
  return params;
}

MappingReporter::MappingReporter(const InputParameters & parameters)
  : StochasticReporter(parameters),
    MappingInterface(this),
    _parallel_storage(isParamValid("parallel_storage")
                          ? &getUserObject<ParallelSolutionStorage>("parallel_storage")
                          : nullptr),
    _sampler(isParamValid("sampler") ? &getSampler("sampler") : nullptr),
    _mapping_name(getParam<UserObjectName>("mapping")),
    _variable_names(getParam<std::vector<VariableName>>("variables"))
{
  if (_parallel_storage)
  {
    if (_sampler)
    {
      // Declaring the reporters for every variable. This is a collection of vectors which describe
      // the coordinates of the solution fields in the latent space.
      _vector_real_values_parallel_storage.resize(_variable_names.size());
      for (auto var_i : index_range(_variable_names))
      {
        _vector_real_values_parallel_storage[var_i] = &declareStochasticReporter<std::vector<Real>>(
            _variable_names[var_i] + "_" + _mapping_name, *_sampler);
      }
    }
    else
      paramError("sampler",
                 "We need a sampler object if the parallel storage is supplied! The sampler object "
                 "shall be the same as the one used to generate the solution fields in the "
                 "parallel storage object.");
  }
  else
  {
    // Declaring the reporters for every variable. This is a collection of vectors which describe
    // the coordinates of the solution fields in the latent space.
    _vector_real_values.resize(_variable_names.size());
    for (auto var_i : index_range(_variable_names))
      _vector_real_values[var_i] = &declareValueByName<std::vector<Real>>(
          _variable_names[var_i] + "_" + _mapping_name, REPORTER_MODE_ROOT);
  }
}

void
MappingReporter::initialSetup()
{
  _mapping = &getMappingByName(_mapping_name);
}

void
MappingReporter::execute()
{
  // We have two execution modes. If the parallel storage is supplied we loop over the snapshots in
  // the parallel storage, and project them to obtain their coefficients.
  if (_parallel_storage)
    mapParallelStorageData();
  // The alternative option is to just fetch the variables and reduce them
  else
    mapVariableData();
}

void
MappingReporter::mapParallelStorageData()
{
  // If the mapping is not built yet, we shall build it using the solutions in the parallel
  // storage. The conditional process is decided in the mapping object.
  for (const auto & var : _variable_names)
    _mapping->buildMapping(var);

  // Since the solution fields can be distributed among the processes of each sub-application
  // (unlike samples and reporter values which are just on the root process), we need to use the
  // information in the sampler to check which solution is where.
  const auto rank_config = _sampler->getRankConfig(true);

  // We need to do some parallel communication in case we have snapshots on processes other than
  // the roots of the sub-applications. This will collect the vectors that we need to send in the
  // following format:
  // <to which processor, (variable index, global sample index, solution field)>
  std::unordered_map<processor_id_type,
                     std::vector<std::tuple<unsigned int, unsigned int, std::vector<Real>>>>
      send_map;

  // We need to use the rank config here to ensure we are looping on the non-root processors
  // too
  for (const auto sample_i : make_range(rank_config.num_local_sims))
  {
    std::vector<Real> data = _sampler->getNextLocalRow();

    // Converting the local indexing to global sample indices
    const unsigned int global_i = sample_i + _sampler->getLocalRowBegin();

    for (const auto var_i : index_range(_variable_names))
    {
      // Create a temporary storage for the coordinates in the latent space
      std::vector<Real> local_vector;

      // Check if the current process has this global sample
      if (_parallel_storage->hasGlobalSample(global_i, _variable_names[var_i]))
      {
        // Fetch the solution vector for the given sample index and variable
        const auto & full_vector =
            _parallel_storage->getGlobalSample(global_i, _variable_names[var_i]);

        // At the moment we only support simulations which have only one solution field
        // per sample. This is typically a steady-state simulation.
        if (full_vector.size() != 1)
          mooseError("MappingReporter is only supported for simulations with one solution "
                     "field per run!");

        // We use the mapping object to generate the coordinates in the latent space
        _mapping->map(_variable_names[var_i], global_i, local_vector);

        // If we are on the root processor of the sub-application, we simply insert the result
        // into the reporter storage space. Othervise we will send it to the root process.
        if (rank_config.is_first_local_rank)
          (*_vector_real_values_parallel_storage[var_i])[sample_i] = local_vector;
        else
          send_map[rank_config.my_first_rank].emplace_back(
              var_i, sample_i, std::move(local_vector));
      }
    }
  }

  // This functor describes what we do when we receive the samples from other processes
  auto receive_functor =
      [this](processor_id_type /*pid*/,
             const std::vector<std::tuple<unsigned int, unsigned int, std::vector<Real>>> & vectors)
  {
    // We unpack the tuples and insert the values into the reporter
    for (const auto & [var_i, sample_i, vector] : vectors)
      (*_vector_real_values_parallel_storage[var_i])[sample_i] = std::move(vector);
  };

  // We send the results from the non-root processors to the root processors
  Parallel::push_parallel_packed_range(_communicator, send_map, (void *)nullptr, receive_functor);
}

void
MappingReporter::mapVariableData()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();

  for (unsigned int var_i = 0; var_i < _variable_names.size(); ++var_i)
  {
    // Getting the corresponding DoF indices for the variable.
    nl.setVariableGlobalDoFs(_variable_names[var_i]);

    // We need to serialize the solution on the root process first.
    DenseVector<Real> serialized_solution;
    nl.solution().localize(serialized_solution.get_values(),
                           processor_id() == 0 ? nl.getVariableGlobalDoFs()
                                               : std::vector<dof_id_type>());
    // We map the solution into the latent space and save the solutions in one go
    if (processor_id() == 0)
      _mapping->map(_variable_names[var_i], serialized_solution, *_vector_real_values[var_i]);
  }
}
