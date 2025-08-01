//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "RhieChowMassFlux.h"
#include "SIMPLESolveBase.h"

/**
 * Common base class for segregated solvers for the Navier-Stokes
 * equations with linear FV assembly routines. Once the nonlinear
 * assembly-based routines are retired, this will be the primary base class
 * instead of SIMPLESolveBase.
 */
class LinearAssemblySegregatedSolve : public SIMPLESolveBase
{
public:
  LinearAssemblySegregatedSolve(Executioner & ex);

  static InputParameters validParams();

  virtual void linkRhieChowUserObject() override;

  /**
   * Performs the momentum pressure coupling.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  /// Return pointers to the systems which are solved for within this object
  const std::vector<LinearSystem *> systemsToSolve() const { return _systems_to_solve; }

protected:
//  std::vector<unsigned int> generateBoundaryMask(const BoundaryID bid, const MooseVariableScalar);

  virtual std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor() override;
  virtual std::pair<unsigned int, Real> solvePressureCorrector() override;

  /// Computes new velocity field based on computed pressure gradients
  /// @param subtract_updated_pressure If we need to subtract the updated
  /// pressure gradient from the right hand side of the system
  /// @param recompute_face_mass_flux If we want to recompute the face flux too
  /// @param solver_params Dummy solver parameter object for the linear solve
  virtual std::pair<unsigned int, Real> correctVelocity(const bool subtract_updated_pressure,
                                                        const bool recompute_face_mass_flux,
                                                        const SolverParams & solver_params);

  /// Solve an equation which contains an advection term that depends
  /// on the solution of the segregated Navier-Stokes equations.
  /// @param system_num The number of the system which is solved
  /// @param system Reference to the system which is solved
  /// @param relaxation_factor The relaxation factor for matrix relaxation
  /// @param solver_config The solver configuration object for the linear solve
  /// @param abs_tol The scaled absolute tolerance for the linear solve
  /// @param field_relaxation (optional) The relaxation factor for fields if relax_fields is true. Default value is 1.0.
  /// @param min_value_limiter (optional) The minimum value for the solution field
  /// @return The normalized residual norm of the equation.
  std::pair<unsigned int, Real>
  solveAdvectedSystem(const unsigned int system_num,
                      LinearSystem & system,
                      const Real relaxation_factor,
                      libMesh::SolverConfiguration & solver_config,
                      const Real abs_tol,
                      const Real field_relaxation = 1.0,
                      const Real min_value_limiter = std::numeric_limits<Real>::min());

  /// Solve an equation which contains the solid energy conservation.
  std::pair<unsigned int, Real> solveSolidEnergy();

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<LinearSystem *> _momentum_systems;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// Reference to the nonlinear system corresponding to the pressure equation
  LinearSystem & _pressure_system;

  /// The number of the system corresponding to the energy equation
  const unsigned int _energy_sys_number;

  /// Pointer to the nonlinear system corresponding to the fluid energy equation
  LinearSystem * _energy_system;

  /// The number of the system corresponding to the solid energy equation
  const unsigned int _solid_energy_sys_number;

  /// Pointer to the nonlinear system corresponding to the solid energy equation
  LinearSystem * _solid_energy_system;

  /// Pointer(s) to the system(s) corresponding to the passive scalar equation(s)
  std::vector<LinearSystem *> _passive_scalar_systems;

  /// Pointer(s) to the system(s) corresponding to the active scalar equation(s)
  std::vector<LinearSystem *> _active_scalar_systems;

  /// Pointer(s) to the system(s) corresponding to the turbulence equation(s)
  std::vector<LinearSystem *> _turbulence_systems;

  /// Pointer to the segregated RhieChow interpolation object
  RhieChowMassFlux * _rc_uo;

  /// Shortcut to every linear system that we solve for here
  std::vector<LinearSystem *> _systems_to_solve;

  // ************************ Active Scalar Variables ************************ //

  /// The names of the active scalar systems
  const std::vector<SolverSystemName> & _active_scalar_system_names;

  /// Boolean for easy check if a active scalar systems shall be solved or not
  const bool _has_active_scalar_systems;

  // The number(s) of the system(s) corresponding to the active scalar equation(s)
  std::vector<unsigned int> _active_scalar_system_numbers;

  /// The user-defined relaxation parameter(s) for the active scalar equation(s)
  const std::vector<Real> _active_scalar_equation_relaxation;

  /// Options which hold the petsc settings for the active scalar equation(s)
  Moose::PetscSupport::PetscOptions _active_scalar_petsc_options;

  /// Options for the linear solver of the active scalar equation(s)
  SIMPLESolverConfiguration _active_scalar_linear_control;

  /// Absolute linear tolerance for the active scalar equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _active_scalar_l_abs_tol;

  /// The user-defined absolute tolerance for determining the convergence in active scalars
  const std::vector<Real> _active_scalar_absolute_tolerance;

  /// number of CHT fixed point iterations.
  const unsigned int _num_cht_fpi;

  /// Tolerance to which temperature and heat flux at the CHT interface must converge during fixed-
  /// point iterations (TODO: separate tols for q and T)
  const Real _cht_fpi_tolerance;
};
