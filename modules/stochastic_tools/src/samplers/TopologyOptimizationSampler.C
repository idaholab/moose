//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TopologyOptimizationSampler.h"
#include "TopologicalConstraintBase.h"
#include "Transient.h"
#include "ConstantDT.h"

registerMooseObjectAliased("StochasticToolsApp", TopologyOptimizationSampler, "TopologyOptimizer");

InputParameters
TopologyOptimizationSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription(
      "This sampler proposes geometric configurations based off the geometry provided in the "
      "subapp in order to minimize a user-defined cost function.");

  params.addRequiredParam<unsigned int>(
      "num_parallel_proposals",
      "Number of proposals to make and corresponding subApps executed in "
      "parallel.");
  MooseEnum neighbor_selection("combinatorial=1", "combinatorial");
  params.addParam<MooseEnum>(
      "neighbor_selection", neighbor_selection, "The neighbor selection algorithm.");
  params.suppressParameter<unsigned int>("min_procs_per_row");
  params.suppressParameter<unsigned int>("max_procs_per_row");
  params.addRequiredParam<dof_id_type>("configuration_size", "Size of the config");
  params.addParam<std::vector<UserObjectName>>(
      "constraints", "Vector of user object names of type TopologicalConstraintBase");
  params.addParam<unsigned int>("seed", 1, "Random seed");
  params.addRequiredParam<unsigned int>("num_iterations",
                                        "The number of simulated annealing iterations");
  return params;
}

TopologyOptimizationSampler::TopologyOptimizationSampler(const InputParameters & parameters)
  : Sampler(parameters),
    TransientInterface(this),
    UserObjectInterface(this),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    _configuration_size(getParam<dof_id_type>("configuration_size")),
    _neighbor_selection(getParam<MooseEnum>("neighbor_selection")),
    _num_it(getParam<unsigned int>("num_iterations"))
{
  // compute the minimum and maximum ranks per subapp
  if (n_processors() < _num_parallel_proposals)
    paramError("num_parallel_proposals", "Should be smaller than number of mpi ranks");
  _min_procs = n_processors() / _num_parallel_proposals;
  _max_procs = _min_procs;
  if (n_processors() % _num_parallel_proposals != 0)
    _max_procs += 1;

  // TODO: something wrong if n_processors() % _num_parallel_proposals != 0
  if (n_processors() % _num_parallel_proposals != 0)
    mooseError("There is a bug in processor setup when _num_parallel_proposals does not divide "
               "n_processors");

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals);
  setNumberOfCols(_configuration_size);

  // seed the random number generator
  _rnd_gen.seed(getParam<unsigned int>("seed") + processor_id() * processor_id());

  // get constraints
  _constraints = {};
  if (isParamValid("constraints"))
  {
    auto names = getParam<std::vector<UserObjectName>>("constraints");
    _constraints.resize(names.size());
    for (unsigned int j = 0; j < names.size(); ++j)
      _constraints[j] = &getUserObjectByName<TopologicalConstraintBase>(names[j]);
  }

  // get the executioner & ensure it's transient
  Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());
  if (!transient)
    mooseError("Executioner must be Transient.");
  // the last iterations is to rerun the best configuration
  transient->forceNumSteps(_num_it + 1);
}

LocalRankConfig
TopologyOptimizationSampler::constructRankConfig(bool batch_mode) const
{
  auto lrc = rankConfig(
      processor_id(), n_processors(), _num_parallel_proposals, _min_procs, _max_procs, batch_mode);

  if (lrc.num_local_apps > 1)
    mooseError("The number of local apps should at most be 1");

  return lrc;
}

void
TopologyOptimizationSampler::setParametersFromSubapp(const MooseMesh * subapp_mesh)
{
  if (getNumberOfLocalRows() > 1)
    mooseError("The number of local rows should at most be 1");

  // store the pointer of the subapp mesh
  _subapp_mesh = subapp_mesh;

  // local rank info
  auto lrc = getRankConfig(false);

  // set root processor id that owns the subapp that this proc is working on
  std::set<unsigned int> all_root_ranks;
  if (lrc.is_first_local_rank)
    all_root_ranks.insert(processor_id());
  _communicator.set_union(all_root_ranks);

  _root_processor_id = 0;
  for (const auto & p : all_root_ranks)
    if (p > _root_processor_id && p <= processor_id())
      _root_processor_id = p;

  //// compute _offset of the elements owned by this vector in the configuration vectors
  // std::vector<dof_id_type> n_local_elements = {_subapp_mesh->getMesh().n_active_local_elem()};
  //_communicator.allgather(n_local_elements);
  //_offset = 0;
  // for (unsigned int j = _root_processor_id; j < processor_id(); ++j)
  // _offset += n_local_elements[j];

  // ensure that the mesh is replicated
  if (_subapp_mesh->isDistributedMesh())
    mooseError("Topology optimization currently does not work with distributed meshes");

  // set the initial sample & check if _configuration_size is equal to
  // nElem from subapp mesh
  if (_configuration_size != _subapp_mesh->getMesh().n_active_elem())
    mooseError("Wrong configuration size. Should be ", _subapp_mesh->getMesh().n_active_elem());
  _current_configurations.resize(_configuration_size, 0);
  _proposed_configurations.resize(_configuration_size, 0);

  // do a loop over all elements to populate the current population,
  // if the loop is split up over all processors of the subapps,
  // there will be a lot of communication and that communication is probably as bad
  // in comm time as the additional cost of repeating the loop over all elements with
  // extra code
  auto begin = _subapp_mesh->getMesh().active_elements_begin();
  auto end = _subapp_mesh->getMesh().active_elements_end();
  dof_id_type index = 0;
  for (const Elem * elem : as_range(begin, end))
  {
    _current_configurations[index] = static_cast<dof_id_type>(elem->subdomain_id());
    index += 1;
  }
}

void
TopologyOptimizationSampler::acceptProposal()
{
  _current_configurations = _proposed_configurations;
}

void
TopologyOptimizationSampler::proposeAndGetConfiguration(std::vector<dof_id_type> & config)
{
  // if _t_step == 1, we are at the first timestep; for the first timestep
  // use the initial configuration that was transferred from the sub app
  if (_t_step == 1)
  {
    _proposed_configurations = _current_configurations;
    config = _proposed_configurations;
    return;
  }
  else if (_t_step == static_cast<int>(_num_it + 1))
  {
    _proposed_configurations = _best_configurations;
    config = _proposed_configurations;
    return;
  }

  // create a configuration proposal
  proposeConfiguration();
  config = _proposed_configurations;
}

void
TopologyOptimizationSampler::proposeConfiguration()
{
  // make the proposed configuration the current configuration
  // then modify the proposed configurations
  _proposed_configurations = _current_configurations;

  // proposed configurations are created on root procs only
  if (processor_id() == _root_processor_id)
  {
    switch (_neighbor_selection)
    {
      case 1:
      {
        // select the first entry randomly
        dof_id_type fi = randConfigIndex();
        // generate random numbers until the second index has a different configuration
        while (true)
        {
          dof_id_type si = randConfigIndex();
          dof_id_type temp = _proposed_configurations[fi];
          _proposed_configurations[fi] = _proposed_configurations[si];
          _proposed_configurations[si] = temp;
          if (proposalSatisfiesConstraints())
            break;
        }
        break;
      }
      default:
        mooseError("Unknown neighbor_selection", _neighbor_selection);
    }
  }

  // allgather the new proposals and then use the proposed config of your
  // root processor
  std::vector<std::vector<dof_id_type>> send_config_buffer;
  _communicator.allgather(_proposed_configurations, send_config_buffer);
  _proposed_configurations = send_config_buffer[_root_processor_id];
}

bool
TopologyOptimizationSampler::proposalSatisfiesConstraints()
{
  // first check that _proposed_configurations and _proposed_configurations
  // are different
  if (_proposed_configurations == _current_configurations)
  {
    _proposed_configurations = _current_configurations;
    return false;
  }

  for (const auto & p : _constraints)
    if (!p->isConfigAllowed(_proposed_configurations, _subapp_mesh))
    {
      _proposed_configurations = _current_configurations;
      return false;
    }
  return true;
}

dof_id_type
TopologyOptimizationSampler::randConfigIndex() const
{
  if (_root_processor_id != processor_id())
    mooseError("Random numbers should never be requested from a non-root rank.");
  return std::floor(_configuration_size * _rnd_gen.rand());
}

dof_id_type
TopologyOptimizationSampler::syncedRandConfigIndex() const
{
  // pull a random number for root procs and communicate them
  dof_id_type ri = 0;
  if (_root_processor_id == processor_id())
    ri = randConfigIndex();
  std::vector<dof_id_type> buffer;
  _communicator.allgather(ri, buffer);
  return buffer[_root_processor_id];
}

Real
TopologyOptimizationSampler::syncedRand() const
{
  // pull a random number for root procs and communicate them
  Real rnd = 0;
  if (_root_processor_id == processor_id())
    rnd = _rnd_gen.rand();
  std::vector<Real> buffer;
  _communicator.allgather(rnd, buffer);
  return buffer[_root_processor_id];
}

Real
TopologyOptimizationSampler::computeSample(dof_id_type /*row_index*/, dof_id_type /*col_index*/)
{
  mooseError("This method should not be called.");
  return 0;
}
