//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "TopologyOptimizationSampler.h"

class Function;

/**
 * Reporter that makes the accept/reject decisions on configurations
 * proposed by the TopologyOptimizationSampler
 */
class TopologyOptimizerDecision : public GeneralReporter
{
public:
  static InputParameters validParams();
  TopologyOptimizerDecision(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;
  virtual void timestepSetup() override;

  ///@{ public interface
  void getProposedConfiguration(unsigned int row, std::vector<dof_id_type> & configuration) const;
  unsigned int numParallelProposals() { return _num_parallel_proposals; }
  void setProposedObjectiveValues(Real objective_val);
  ///@}

protected:
  /// called in execute, accepts or rejects proposals and checks for best configuration
  void acceptOrRejectProposal();

  /// pointer to the topological optimization sampler
  TopologyOptimizationSampler * _top_opt;

  /// The number of parallel Markov chains that are independently optiized
  dof_id_type _num_parallel_proposals;

  /// the configuration that was just proposed by the sampler
  std::vector<dof_id_type> _proposed_configuration;

  /// the objective value of the proposed configuration
  Real _proposed_objective_value;

  /// the current configuration that was used to create the proposed configuration
  std::vector<dof_id_type> _current_configuration;

  /// the objective value of the current configuration
  Real _current_objective_value;

  /// the best configuration that this object has seen
  std::vector<dof_id_type> _best_configuration;

  /// the objective value of the best configuration
  Real _best_objective_value;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;

  /// a function given the simulated annealing temperature as function of iteration
  const Function & _temperature_schedule;

  /// if the decision after each iteration is printed or not
  bool _print_decisions;

  /// the chains of configurations are reset to the best configuration [over all processors] every _reset_frequency steps
  unsigned int _reset_frequency;
};
