//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"

class TopologicalConstraintBase;

/**
 * This sampler proposes geometric configurations based off the geometry
 * provided in the subapp in order to minimize a user-defined cost function.
 */
class TopologyOptimizationSampler : public Sampler,
                                    public TransientInterface,
                                    public UserObjectInterface
{
public:
  static InputParameters validParams();

  TopologyOptimizationSampler(const InputParameters & parameters);

  /// Return the number of parallel proposals.
  dof_id_type getNumParallelProposals() const { return _num_parallel_proposals; }

  // called by InitialTopologicalMeshTransfer to set initial configuration using subapp data
  void setParametersFromSubapp(const MooseMesh * subapp_mesh);

  /// proposes a new configuration and then returns it via config parameter
  void proposeAndGetConfiguration(std::vector<dof_id_type> & config);

  /// called by the topological decision reporter to copy over proposed config to current config
  void acceptProposal();

  /// returns the current configuration as config
  void getCurrentConfiguration(std::vector<dof_id_type> & config) const
  {
    config = _current_configurations;
  }
  /// sets the current configuration to config
  void setCurrentConfiguration(const std::vector<dof_id_type> & config)
  {
    _current_configurations = config;
  }
  /// gets the proposed configuration as config
  void getProposedConfiguration(std::vector<dof_id_type> & config) const
  {
    config = _proposed_configurations;
  }
  /// sets the best configuration to config
  void setBestConfiguration(const std::vector<dof_id_type> & config)
  {
    _best_configurations = config;
  }
  /// returns the number of iterations
  unsigned int numIt() const { return _num_it; }

  /// returns a randomly selected dof_id_type between 0 and the configuration size, should only be called from subapp root proc
  dof_id_type randConfigIndex() const;

  /**
   * This is the synced version of randConfigIndex(). This means that all procs belonging to the
   * same subapp get the same random number Do not call this in code portions that are not traversed
   * by ALL processors or it will hang
   */
  dof_id_type syncedRandConfigIndex() const;

  /**
   * Returns a random real between 0 and 1 and is synced.
   * This means that all procs belonging to the same subapp get the same random number
   * Do not call this in code portions that are not traversed by ALL processors or it will hang
   */
  Real syncedRand() const;

  /**
   * returns the local root processor
   * [Note: for each subapp multiple procs might be assigned but one proc owns the subapp,
   *  that is the root]
   */
  unsigned int rootProc() const { return _root_processor_id; }

protected:
  /// proposes a new configuration and stores it in _proposed_configurations
  void proposeConfiguration();

  /// this function returns true of proposal satisfies all constraints and RESETS proposed config to current config if not and returns false
  bool proposalSatisfiesConstraints();

  /// we override this function to set up subapp partitioning in an appropriate way
  virtual LocalRankConfig constructRankConfig(bool batch_mode) const override;

  /// override this method to make class concrete
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Number of parallel proposals to be made and subApps to be executed
  const unsigned int _num_parallel_proposals;

  /// The size of the configuration, usually the number of elements in the subapp domain
  const dof_id_type _configuration_size;

  /// the neighbor selection algorithm type
  MooseEnum _neighbor_selection;

  /// the number of iterations
  unsigned int _num_it;

  /// a MOOSE random number generator object that is seeded differently for each processor
  MooseRandom _rnd_gen;

  /// a vector of TopologicalConstraintBase that are checked by proposalSatisfiesConstraints()
  std::vector<const TopologicalConstraintBase *> _constraints;

  /// the current configuration [i.e. has been accepted in prior iterations]
  std::vector<dof_id_type> _current_configurations;

  /// the new, proposed configuration
  std::vector<dof_id_type> _proposed_configurations;

  /// the best configuration
  std::vector<dof_id_type> _best_configurations;

  /// The rank of the processor that is root for the local subapp
  unsigned int _root_processor_id;

  ///@{ minimum and maximum number of processor per subapp and Markov chain of configurations
  unsigned int _min_procs;
  unsigned int _max_procs;
  ///@}
  // unsigned int _offset;

  /// a convenience pointer to the mesh of the subapp that this processor works on
  const MooseMesh * _subapp_mesh;
};
