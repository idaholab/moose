/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "RandomData.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "RandomInterface.h"

const unsigned int MASTER = std::numeric_limits<unsigned int>::max();

RandomData::RandomData(FEProblem &problem, const RandomInterface & random_interface) :
    _rd_problem(problem),
    _rd_mesh(problem.mesh()),
    _is_nodal(random_interface.isNodal()),
    _reset_on(random_interface.getResetOnTime()),
    _master_seed(random_interface.getMasterSeed()),
    _current_master_seed(0),
    _new_seed(1)
{
}

unsigned int
RandomData::getSeed(unsigned int id)
{
  mooseAssert(_seeds.find(id) != _seeds.end(), "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls");

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
   * case EXEC_TIMESTEP:         // reset and advance every timestep
   * case EXEC_RESIDUAL:         // Reset every residual, advance every timestep
   * case EXEC_JACOBIAN:         // Reset every Jacobian, advance every timestep
   */

  // If the _new_seed has been updated, we need to update all of the generators
  if (_new_seed != _current_master_seed)
  {
    _current_master_seed = _new_seed;
    updateGenerators();
    _generator.saveState();       // Save states so that we can reset on demand
  }

  if (_reset_on == exec_flag)
    _generator.restoreState();    // Restore states here
}

void
RandomData::updateGenerators()
{
  {
    /**
     * Set the master seed and repopulate all of the child generators
     */
    _generator.seed(MASTER, _current_master_seed);

    if (_is_nodal)
    {
      MeshBase::const_node_iterator it = _rd_mesh.getMesh().active_nodes_begin();
      MeshBase::const_node_iterator end_it = _rd_mesh.getMesh().active_nodes_end();

      for (; it != end_it; ++it)
      {
        unsigned int id = (*it)->id();
        _seeds[id] = _generator.randl(MASTER);

        // Update the individual dof object generators
        _generator.seed(id, _seeds[id]);
      }
    }
    else
    {
      MeshBase::const_element_iterator it = _rd_mesh.getMesh().active_elements_begin();
      MeshBase::const_element_iterator end_it = _rd_mesh.getMesh().active_elements_end();

      for (; it != end_it; ++it)
      {
        unsigned int id = (*it)->id();
        _seeds[id] = _generator.randl(MASTER);

        // Update the individual dof object generators
        _generator.seed(id, _seeds[id]);
      }
    }
  }
}

