//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TopologyOptimizerDecision.h"
#include "Sampler.h"
#include "DenseMatrix.h"
#include "Function.h"
#include <limits.h>

registerMooseObject("StochasticToolsApp", TopologyOptimizerDecision);

InputParameters
TopologyOptimizerDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter that makes the accept/reject decisions on configurations "
                             "proposed by the TopologyOptimizationSampler.");
  params.addRequiredParam<FunctionName>(
      "temperature_schedule", "The name of the function providing the temperature schedule");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  // this object should be executed on timestep end
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  params.addParam<bool>(
      "print_decisions", false, "If the object prints acceptance/rejection decisions");
  params.addParam<unsigned int>(
      "reset_frequency",
      std::numeric_limits<unsigned int>::max(),
      "The number of iterations before the chain resets to the best sample.");
  return params;
}

TopologyOptimizerDecision::TopologyOptimizerDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _top_opt(dynamic_cast<TopologyOptimizationSampler *>(&getSampler("sampler"))),
    _local_comm(getSampler("sampler").getLocalComm()),
    _temperature_schedule(getFunction("temperature_schedule")),
    _print_decisions(getParam<bool>("print_decisions")),
    _reset_frequency(getParam<unsigned int>("reset_frequency"))
{
  // Check whether the selected sampler is an MCMC sampler or not
  if (!_top_opt)
    paramError("sampler", "The selected sampler is not of type MCMC.");

  // Fetching the sampler characteristics
  _num_parallel_proposals = _top_opt->getNumParallelProposals();
}

void
TopologyOptimizerDecision::timestepSetup()
{
  // get the proposed sample at the beginning of each timestep
  // on timestep 1, the TopologyOptimizationSampler uses the configuration
  // that is provided in the subapp input file
  _top_opt->proposeAndGetConfiguration(_proposed_configuration);
}

void
TopologyOptimizerDecision::execute()
{
  // this is where the decision on accepting or rejecting the
  // _proposed_configurations are made

  // on the first timestep, we have only run the _proposed_configuration
  // so we simply accept it and set it as the _current_configuration and
  // set the current objective value and best objective value
  // to the proposed objective value
  if (_t_step == 1)
  {
    _current_configuration = _proposed_configuration;
    _current_objective_value = _proposed_objective_value;
    _best_objective_value = _proposed_objective_value;
    return;
  }

  acceptOrRejectProposal();
}

void
TopologyOptimizerDecision::acceptOrRejectProposal()
{
  // Decide whether to accept the proposed sample
  // Note: all processors execute this code; the syncedRand()
  // function ensures that processors belonging to the same
  // root proc get the same random number and will therefore
  // make the same decision
  Real synced_randn = _top_opt->syncedRand();
  unsigned int accepted_proposal = 0;
  Real old_objective_value = _current_objective_value;
  Real transition_probability =
      _proposed_objective_value < _current_objective_value
          ? 1
          : std::exp((_current_objective_value - _proposed_objective_value) /
                     _temperature_schedule.value((Real)_t_step));
  if (synced_randn < transition_probability)
  {
    _current_configuration = _proposed_configuration;
    _current_objective_value = _proposed_objective_value;
    _top_opt->acceptProposal();
    accepted_proposal = 1;
  }

  // Update the best configuration
  // This will happen for each app separately, so each app
  // (aka a set of procs associated with a common root proc)
  // may have a different best configuration
  // This will be resolved at the handshake where we reset
  // to best configuration
  if (_proposed_objective_value < _best_objective_value)
  {
    _best_objective_value = _proposed_objective_value;
    _best_configuration = _proposed_configuration;
    _top_opt->setBestConfiguration(_best_configuration);
  }

  // We reset every _reset_frequency steps
  // This requires a handshake between all groups of procs
  // to find the single best configuration
  if (_t_step == static_cast<int>(_top_opt->numIt()) ||
      ((_t_step - 1) % _reset_frequency == 0 && _t_step != static_cast<int>(_top_opt->numIt()) + 1))
  {
    // communication of best objective values AND configurations
    std::vector<Real> best_objective_value_buffer;
    _communicator.allgather(_best_objective_value, best_objective_value_buffer);
    std::vector<std::vector<dof_id_type>> best_config_buffer;
    _communicator.allgather(_best_configuration, best_config_buffer);

    // find best of the best configuration index
    unsigned int min_index = 0;
    Real best_value = std::numeric_limits<Real>::max();
    for (unsigned int j = 0; j < best_objective_value_buffer.size(); ++j)
      if (best_objective_value_buffer[j] < best_value)
      {
        best_value = best_objective_value_buffer[j];
        min_index = j;
      }

    // set best objective value and config and update the sampler
    _current_configuration = best_config_buffer[min_index];
    _current_objective_value = best_value;
    _best_configuration = best_config_buffer[min_index];
    _best_objective_value = best_value;
    _top_opt->setCurrentConfiguration(_current_configuration);
    _top_opt->setBestConfiguration(_current_configuration);
    if (_t_step == static_cast<int>(_top_opt->numIt()))
      _console
          << "Preparing for last iteration. Resetting current configuration to best configuration"
          << std::endl;
    else
      _console << "Resetting current configuration to best configuration" << std::endl;
    _console << "Best objective value: " << _best_objective_value << std::endl;
  }
  else if (_print_decisions)
  {
    // this information will only get printed if we have not resorted to the best value
    std::vector<unsigned int> accepted_proposal_buffer;
    std::vector<Real> old_objective_value_buffer;
    std::vector<Real> proposed_objective_value_buffer;
    std::vector<unsigned int> root_proc_buffer;
    std::vector<Real> transition_probability_buffer;
    _communicator.gather(0, accepted_proposal, accepted_proposal_buffer);
    _communicator.gather(0, old_objective_value, old_objective_value_buffer);
    _communicator.gather(0, _top_opt->rootProc(), root_proc_buffer);
    _communicator.gather(0, _proposed_objective_value, proposed_objective_value_buffer);
    _communicator.gather(0, transition_probability, transition_probability_buffer);
    if (processor_id() == 0)
      for (unsigned int j = 0; j < n_processors(); ++j)
      {
        if (j != root_proc_buffer[j])
          continue;

        if (accepted_proposal_buffer[j] == 1)
          _console << "Rank " << j
                   << " accepted proposal with cost: " << proposed_objective_value_buffer[j]
                   << " vs. previous cost: " << old_objective_value_buffer[j]
                   << ", transition probability " << transition_probability_buffer[j] << std::endl;
        else
          _console << "Rank " << j
                   << " rejected proposal with cost: " << proposed_objective_value_buffer[j]
                   << " vs. previous cost: " << old_objective_value_buffer[j]
                   << ", transition probability " << transition_probability_buffer[j] << std::endl;
      }
  }
}

void
TopologyOptimizerDecision::getProposedConfiguration(unsigned int /*row*/,
                                                    std::vector<dof_id_type> & configuration) const
{
  configuration = _proposed_configuration;
}

void
TopologyOptimizerDecision::setProposedObjectiveValues(Real objective_val)
{
  _proposed_objective_value = objective_val;
}
