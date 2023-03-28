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
  virtual void initializeFromMultiapp() override;
  virtual void executeFromMultiapp() override;
  virtual void finalizeFromMultiapp() override;

  virtual void initializeToMultiapp() override;
  virtual void executeToMultiapp() override;
  virtual void finalizeToMultiapp() override;
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
  void transferToRoot(NonlinearSystemBase & app_nl_system, SolutionContainer & solution_container);

  /// Serialize on methodically determined rank of the subapp and transfer to the main application
  void transferInParallel(NonlinearSystemBase & app_nl_system,
                          SolutionContainer & solution_container);

  /// User determined switch that determines if we want to serialize on root only or distribute the
  /// solutions between all the ranks of the subapp
  const bool _serialize_on_root;
};
