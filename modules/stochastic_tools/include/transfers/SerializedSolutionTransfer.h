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

/**
 * This class is responsible for serializing solutions coming from subapps on
 * specific processors. It is designed to serve as an interface between
 * SolutionContainer and ParallelSolutionStorage objects.
 */
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
  void initializeFromMultiapp() override {}
  void executeFromMultiapp() override;
  void finalizeFromMultiapp() override {}

  void initializeToMultiapp() override {}
  void executeToMultiapp() override {}
  void finalizeToMultiapp() override {}
  ///@}

protected:
  /// The storage on the main application where the serialized solutions should be
  /// transferred
  ParallelSolutionStorage * _parallel_storage;

  /// The names of the variables which should be extracted from the solution vector
  std::vector<VariableName> _variable_names;

  /// Link to the storage spaces on the subapplications (will only hold one in batch mode)
  std::vector<SolutionContainer *> _solution_container;

private:
  /// Serialize on the root processor of the subapplication and transfer the result to the main application
  void transferToSubAppRoot(NonlinearSystemBase & app_nl_system,
                            SolutionContainer & solution_container,
                            const dof_id_type global_i);

  /**
   * Serialize on methodically determined rank of the subapp and transfer to the main application.
   * Example: Let's say we have 5 samples and 3 processors on a sub-application.
   * In this case, we will serialize the first two on rank 1, the second two on rank
   * 2 and the last one on rank 3.
   */
  void transferInParallel(NonlinearSystemBase & app_nl_system,
                          SolutionContainer & solution_container,
                          const dof_id_type global_i);

  /**
   * Initializes the solution container if the multiapp is run in normal mode. We need this because
   * in normal mode we don't have a function for initialization besides `initialSetup()`, which
   * is execute every time regardless of the multiapp settings.
   */
  void initializeInNormalMode();

  /// This routine queries the solution container addresses from the subapps. We need to redo this
  /// every time initialSetup() (batch-reset) is called on the subapp because the address
  /// of SolutionContainer changes. Considering that the transfer doesn't know the multiapp
  /// setting, we use the same approach for batch-restore as well, which might be a little
  /// wasteful if the execution of the subapps is very fast (usually not the case).
  void initializeInBatchMode();

  /// User-selected switch that determines if we want to serialize on the root of the subapp
  /// only or distribute the solutions between all the ranks of the subapp.
  const bool _serialize_on_root;
};
