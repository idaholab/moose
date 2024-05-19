//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SegregatedSolverBase.h"
#include "INSFVRhieChowInterpolatorSegregated.h"
#include "PetscSupport.h"
#include "SolverParams.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/equation_systems.h"
#include "libmesh/solver_configuration.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Executioner set up to solve a thermal-hydraulics problem using the
 * SIMPLENonlinearAssembly algorithm. It utilizes segregated linearized systems
 * which are solved using a fixed-point iteration.
 */
class SIMPLENonlinearAssembly : public SegregatedSolverBase
{
public:
  static InputParameters validParams();

  SIMPLENonlinearAssembly(const InputParameters & parameters);

  virtual void init() override;
  virtual void execute() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

  const INSFVRhieChowInterpolatorSegregated & getRCUserObject() { return *_rc_uo; }

protected:
  /// Solve a momentum predictor step with a fixed pressure field
  /// @return A vector for the normalized residual norms of the momentum equations.
  ///         The length of the vector equals the dimensionality of the domain.
  std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor();

  /// Solve a pressure corrector step.
  /// @return The normalized residual norm of the pressure equation.
  std::pair<unsigned int, Real> solvePressureCorrector();

  /// Solve an equation which contains an advection term that depends
  /// on the solution of the segregated Navier-Stokes equations.
  /// @param system_num The number of the system which is solved
  /// @param system Reference to the system which is solved
  /// @param relaxation_factor The relaxation factor for matrix relaxation
  /// @param solver_config The solver configuration object for the linear solve
  /// @param abs_tol The scaled absolute tolerance for the linear solve
  /// @return The normalized residual norm of the equation.
  std::pair<unsigned int, Real> solveAdvectedSystem(const unsigned int system_num,
                                                    NonlinearSystemBase & system,
                                                    const Real relaxation_factor,
                                                    SolverConfiguration & solver_config,
                                                    const Real abs_tol);

  /// Solve the solid energy conservation equation.
  /// @return The normalized residual norm of the solid equation.
  std::pair<unsigned int, Real> solveSolidEnergySystem();

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// The number of the system corresponding to the energy equation
  const unsigned int _energy_sys_number;

  /// The number of the system corresponding to the solid energy equation
  const unsigned int _solid_energy_sys_number;

  /// The number(s) of the system(s) corresponding to the passive scalar equation(s)
  std::vector<unsigned int> _passive_scalar_system_numbers;

  /// The number(s) of the system(s) corresponding to the turbulence equation(s)
  std::vector<unsigned int> _turbulence_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<NonlinearSystemBase *> _momentum_systems;

  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_system;

  /// Pointer to the nonlinear system corresponding to the fluid energy equation
  NonlinearSystemBase * _energy_system;

  /// Pointer to the nonlinear system corresponding to the solid energy equation
  NonlinearSystemBase * _solid_energy_system;

  /// Pointer(s) to the system(s) corresponding to the passive scalar equation(s)
  std::vector<NonlinearSystemBase *> _passive_scalar_systems;

  /// Pointer(s) to the system(s) corresponding to the turbulence equation(s)
  std::vector<NonlinearSystemBase *> _turbulence_systems;

  /// Pointer to the segregated RhieChow interpolation object
  INSFVRhieChowInterpolatorSegregated * _rc_uo;

private:
  /**
   * Check if the system contains a time kernel or not. This is a steady-state executioner so we
   * want to make sure the user doesn't have kernels that add time-derivatives.
   * @param system Reference to the system which holds the kernels. This is not const because
   * the containsTimeKernel() routine is not const.
   */
  void checkIntegrity(NonlinearSystemBase & system);

  /// The name of the vector tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const TagName _pressure_tag_name;

  /// The ID of the tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const TagID _pressure_tag_id;
};
