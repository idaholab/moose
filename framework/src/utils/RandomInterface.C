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

#include "Moose.h"
#include "RandomInterface.h"
#include "FEProblem.h"
#include "Assembly.h"
#include "MooseMesh.h"

const unsigned int MASTER = std::numeric_limits<unsigned int>::max();

template<>
InputParameters validParams<RandomInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<unsigned int>("seed", 0, "The seed for the master random number generator");

  params.addParamNamesToGroup("seed", "Advanced");
  return params;
}

RandomInterface::RandomInterface(InputParameters & parameters, FEProblem & problem, THREAD_ID tid, bool is_nodal) :
    _ri_problem(problem),
    _ri_assembly(problem.assembly(tid)),
    _ri_mesh(problem.mesh()),
    _ri_tid(tid),
    _master_seed(parameters.get<unsigned int>("seed")),
    _is_nodal(is_nodal),
    _current_master_seed(0),
    _reset_on(EXEC_RESIDUAL),
    _curr_node(_ri_assembly.node()),
    _curr_element(_ri_assembly.elem())
{
}

RandomInterface::~RandomInterface()
{
}

void
RandomInterface::setRandomResetFrequency(ExecFlagType exec_flag)
{
  _reset_on = exec_flag;
  _ri_problem.registerRandomInterface(this, _ri_tid);
}

void
RandomInterface::updateMasterSeed(unsigned int seed)
{
  _current_master_seed = seed;
}

void
RandomInterface::updateSeeds(ExecFlagType exec_flag)
{
  /**
   * Set the seed. This part is critical! If this is done incorrectly, it may lead to difficult to
   * detect patterns in your random numbers either within a single run or over the course of
   * several runs.  We will default to _master_seed + the current time step.
   */
  if (exec_flag == EXEC_INITIAL)
    _current_master_seed = _master_seed;
  else
    _current_master_seed = _master_seed + _ri_problem.timeStep();
  /**
   * case EXEC_TIMESTEP_BEGIN:   // reset and advance every timestep
   * case EXEC_TIMESTEP:         // reset and advance every timestep
   * case EXEC_RESIDUAL:         // Reset every residual, advance every timestep
   * case EXEC_JACOBIAN:         // Reset every Jacobian, advance every timestep
   */

  if (_reset_on == exec_flag            // If the exec_flag matches the reset flag
      || (exec_flag == EXEC_INITIAL)    // or if this is the first time...
      || (_reset_on != EXEC_INITIAL &&  // or if it's at least reset_on timestep or faster
          (exec_flag == EXEC_TIMESTEP || exec_flag == EXEC_TIMESTEP_BEGIN))) // and we are at a timestep
  {
    /**
     * Set the master seed and repopulate all of the child generators
     */
    _generator.seed(MASTER, _current_master_seed);

    if (_is_nodal)
    {
      MeshBase::const_node_iterator it = _ri_mesh.getMesh().active_nodes_begin();
      MeshBase::const_node_iterator end_it = _ri_mesh.getMesh().active_nodes_end();

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
      MeshBase::const_element_iterator it = _ri_mesh.getMesh().active_elements_begin();
      MeshBase::const_element_iterator end_it = _ri_mesh.getMesh().active_elements_end();

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

unsigned int
RandomInterface::getSeed(unsigned int id)
{
  mooseAssert(_seeds.find(id) != _seeds.end(), "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls");

  return _seeds[id];
}

unsigned long
RandomInterface::getRandomLong()
{
  if (_is_nodal)
  {
    mooseAssert(_seeds.find(_curr_node->id()) != _seeds.end(), "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls.");
    return _generator.randl(_curr_node->id());
  }
  else
  {
    mooseAssert(_seeds.find(_curr_element->id()) != _seeds.end(), "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls.");
    return _generator.randl(_curr_element->id());
  }
}

Real
RandomInterface::getRandomReal()
{
  if (_is_nodal)
  {
    mooseAssert(_seeds.find(_curr_node->id()) != _seeds.end(), "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls.");
    return _generator.rand(_curr_node->id());
  }
  else
  {
    mooseAssert(_seeds.find(_curr_element->id()) != _seeds.end(), "Call to updateSeeds() is stale! Check your initialize() or timestepSetup() calls.");
    return _generator.rand(_curr_element->id());
  }
}

