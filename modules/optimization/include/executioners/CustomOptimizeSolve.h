//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"
#include "ExecFlagEnum.h"

class OptimizationReporterBase;
class CustomOptimizationAlgorithm;
class SimulatedAnnealingAlgorithm;
/**
 *
 */
class CustomOptimizeSolve : public SolveObject
{
public:
  static InputParameters validParams();
  CustomOptimizeSolve(Executioner & ex);

  virtual bool solve() override;

  OptimizationReporterBase & getOptimizationReporter() const { return *_obj_function; }
  //const std::vector<Real> & getRealParameters() const { return _real_parameters; }

protected:
  /// Communicator used for operations
  const libMesh::Parallel::Communicator _my_comm;

  /// List of execute flags for when to solve the system
  const ExecFlagEnum & _solve_on;

  /// the optimization algorithm type
  MooseEnum _opt_alg_type;

  /// the optimization algorithm object
  // std::unique_ptr<MooseMesh> _mesh;
  std::unique_ptr<CustomOptimizationAlgorithm> _opt_alg;

  /// objective function defining objective, gradient, and hessian
  OptimizationReporterBase * _obj_function = nullptr;

private:
  ///@{
  /// Function wrappers for tao
  static void objectiveFunctionWrapper(Real & objective,
                                       const std::vector<Real> & rparams,
                                       const std::vector<int> & iparams,
                                       void * ctx);
  ///@}

  //std::vector<Real> _real_parameters;
  //unsigned int _ndof;
};
