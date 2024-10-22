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
#include "PetscSupport.h"
#include "SolveObject.h"
#include "MooseUtils.h"

// Libmesh includes
#include "libmesh/solver_configuration.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Solver configuration class used with the linear solvers in a SIMPLE solver.
 */
class LinearPicardSolverConfiguration : public libMesh::SolverConfiguration
{
  /**
   * Override this to make sure the PETSc options are not overwritten in the linear solver
   */
  virtual void configure_solver() override {}
};

/**
 * Solve object for a fixed point iteration on multiple different linear systems
 */
class LinearFixedPointSolve : public SolveObject
{
public:
  static InputParameters validParams();

  LinearFixedPointSolve(Executioner & ex);

  /**
   * Performs the fixed point iteration between the available linear systems.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

private:

  /// Internal routine for solving the sytems
  std::pair<unsigned int, Real> solveSystem(const unsigned int sys_number, const Moose::PetscSupport::PetscOptions * po);

  /// Vector of linear systems names to be used for the iteration
  const std::vector<LinearSystemName> & _linear_sys_names;

  /// Vector of linear system numbers to be used for the iteration
  std::vector<unsigned int> _linear_sys_numbers;

  /// Petsc options for the different linear systems
  std::vector<Moose::PetscSupport::PetscOptions> _petsc_options;

  /// The allowed maximum number of iterations
  const unsigned int _number_of_iterations;

  /// The tolerances for convergence
  const std::vector<Real> _absolute_tolerances;

  /// If solve should continue if maximum number of iterations is hit
  const bool _continue_on_max_its;

  /// If the operators and vectors should be printed or not
  const bool _print_operators_and_vectors;
};
