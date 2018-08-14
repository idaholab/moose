//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomData.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "RandomInterface.h"

const unsigned int MASTER = std::numeric_limits<unsigned int>::max();

RandomData::RandomData(FEProblemBase & fe_problem, const RandomInterface & random_interface)
  : RandomData(fe_problem,
               random_interface.isNodal(),
               random_interface.getResetOnTime(),
               random_interface.getMasterSeed())
{
}

RandomData::RandomData(FEProblemBase & fe_problem,
                       bool is_nodal,
                       ExecFlagType reset_on,
                       unsigned int seed)
  : _rd_problem(fe_problem),
    _rd_mesh(fe_problem.mesh()),
    _is_nodal(is_nodal),
    _reset_on(reset_on),
    _master_seed(seed),
    _current_master_seed(std::numeric_limits<unsigned int>::max()),
    _new_seed(0)
{
}

unsigned int
RandomData::getSeed(dof_id_type id)
{
  mooseAssert(_seeds.find(id) != _seeds.end(),
              "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls");

  return _seeds[id];
}

void
RandomData::updateSeeds(ExecFlagType exec_flag)
{
  /**
   * Set the seed. This part is critical! If this is done incorrectly, it may lead to difficult to
   * detect patterns in your random numbers either within a single run or over the course of
   * several runs.  We will default to _master_seed + the current time step.
   */
  if (exec_flag == EXEC_INITIAL)
    _new_seed = _master_seed;
  else
    _new_seed = _master_seed + _rd_problem.timeStep();
  /**
   * case EXEC_TIMESTEP_BEGIN:   // reset and advance every timestep
   * case EXEC_TIMESTEP_END:     // reset and advance every timestep
   * case EXEC_LINEAR:           // Reset every residual, advance every timestep
   * case EXEC_NONLINEAR:        // Reset every Jacobian, advance every timestep
   */

  // If the _new_seed has been updated, we need to update all of the generators
  if (_new_seed != _current_master_seed)
  {
    _current_master_seed = _new_seed;
    updateGenerators();
    _generator.saveState(); // Save states so that we can reset on demand
  }

  if (_reset_on == exec_flag)
    _generator.restoreState(); // Restore states here
}

void
RandomData::updateGenerators()
{
  //  Set the master seed and repopulate all of the child generators
  _generator.seed(MASTER, _current_master_seed);

  /**
   * When using parallel mesh it's not worth generating parallel consistent numbers.
   * Each processor may not be aware of which entities belong on another mesh. We do have
   * to be careful to *not* generate the same patterns on different processors however.
   * To do that, we will use the MASTER generator to generate new master seeds for each
   * processor based on their individual processor ids before generating seeds for
   * the mesh entities.
   */
  if (_rd_mesh.isDistributedMesh())
  {
    unsigned int parallel_seed = 0;
    for (processor_id_type proc_id = 0; proc_id < _rd_problem.n_processors(); ++proc_id)
      if (proc_id == _rd_problem.processor_id())
        parallel_seed = _generator.randl(MASTER);
      else
        _generator.randl(MASTER); // Generate but throw away numbers that aren't mine

    _generator.seed(MASTER, parallel_seed);
  }

  auto processor_id = _rd_problem.processor_id();

  if (_is_nodal)
  {
    const auto & node_to_elem = _rd_mesh.nodeToElemMap();
    auto & mesh = _rd_mesh.getMesh();

    for (const auto node_ptr :
         as_range(_rd_mesh.getMesh().active_nodes_begin(), _rd_mesh.getMesh().active_nodes_end()))
    {
      auto id = node_ptr->id();
      auto rand_int = _generator.randl(MASTER);

      // Only save states for nodes attached to active elements
      auto elem_id_it = node_to_elem.find(id);

      if (elem_id_it != node_to_elem.end())
      {
        for (auto elem_id : elem_id_it->second)
        {
          const auto * elem_ptr = mesh.elem_ptr(elem_id);

          if (elem_ptr && processor_id == elem_ptr->processor_id())
          {
            _seeds[id] = rand_int;

            // Update the individual dof object generators
            _generator.seed(id, rand_int);
            break;
          }
        }
      }
    }
  }
  else
  {
    for (const auto & elem_ptr : as_range(_rd_mesh.getMesh().active_elements_begin(),
                                          _rd_mesh.getMesh().active_elements_end()))
    {
      auto id = elem_ptr->id();
      auto rand_int = _generator.randl(MASTER);

      // Only save states for local elements
      if (processor_id == elem_ptr->processor_id())
      {
        _seeds[id] = rand_int;

        // Update the individual dof object generators
        _generator.seed(id, rand_int);
      }
    }
  }
}
