//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ParallelSolutionStorage.h"
#include "StochasticToolsTransfer.h"
#include "SolutionContainer.h"
#include "UserObjectInterface.h"

// Forward declarations
class ParallelSolutionStorage;

class SerializedSolutionTransfer : public StochasticToolsTransfer
{
public:
  static InputParameters validParams();
  SerializedSolutionTransfer(const InputParameters & parameters);
  virtual void initialSetup() override;

  virtual void execute() override;

  ///@{
  /**
   * Methods used when running in batch mode (see SamplerFullSolveMultiApp)
   */
  virtual void initializeFromMultiapp() override;
  virtual void executeFromMultiapp() override;
  virtual void finalizeFromMultiapp() override;

  virtual void initializeToMultiapp() override;
  virtual void executeToMultiapp() override;
  virtual void finalizeToMultiapp() override;

  void fillTransferMap(std::vector<std::pair<unsigned int, unsigned int>> & incoming_range,
                       std::vector<std::pair<unsigned int, unsigned int>> & outgoing_range,
                       std::vector<unsigned int> & app_to_processor,
                       std::map<unsigned int, processor_id_type> & transfer_map);
  void
  createDataPartitioning(std::vector<unsigned int> & new_snapshots_per_app,
                         std::vector<unsigned int> & local_already_in_container,
                         std::vector<std::pair<unsigned int, unsigned int>> & incoming_begin_end,
                         std::vector<std::pair<unsigned int, unsigned int>> & outgoing_begin_end);
  ///@}

protected:
  // bool hasLocalSolution(const std::vector<unsigned int> & solution_distribution,
  //                       const unsigned int & app_index,
  //                       const unsigned int & index);

  /// The input multiapp casted into a PODFullSolveMultiapp to get access to the
  /// specific pod attributes. Used in batch mode only and checking if the
  /// correct MultiApp type has been provided.
  ParallelSolutionStorage * _parallel_storage;

  std::vector<VariableName> _variable_names;

  std::string _serialized_solution_reporter;

  std::vector<processor_id_type> _root_processors;

  unsigned int _num_true_global_apps;

private:
  unsigned int _local_solutions_begin;
  unsigned int _local_solution_end;
  unsigned int _num_global_entries;
  unsigned int _num_local_entries;
};
