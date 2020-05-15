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
#include "MoosePreconditioner.h"

// libMesh includes
#include "libmesh/preconditioner.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/enum_preconditioner_type.h"
#include "libmesh/mesh_base.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/parallel_object.h"

// C++ includes
#include <vector>

// Forward declarations
class NonlinearSystemBase;
class DualMortarPreconditioner;

template <>
InputParameters validParams<DualMortarPreconditioner>();

/**
 * Interface for condensing out LMs for the dual mortar approach.
 */
class DualMortarPreconditioner : public MoosePreconditioner, public Preconditioner<Number>
{
public:
  static InputParameters validParams();

  DualMortarPreconditioner(const InputParameters & params);
  virtual ~DualMortarPreconditioner();

  /**
   * Reconstruct the equation system
   */
  void condenseSystem();

  /**
   * Get dofs for variable in subdomains
   */
  void getDofVarSubdomain();

  /**
   * Get dofs for variable in the interior of subdomains
   */
  void getDofVarInterior();

  /**
   * Get dofs for variable on the contact inferface
   */
  void getDofVarInterface();

  /**
   * Get condensed x and y
   */
  void getCondensedXY(const NumericVector<Number> & y, NumericVector<Number> & x);

  /**
   * Compute Lagrange multipliers using updated solution vector
   */
  void computeLM();

  /**
   * Print dof info for debuggin purposes
   */
  void printDofSets();


  void print_node_info();


  /**
   * Initialize data structures if not done so already.
   */
  virtual void init();

  /**
   * This is called every time the "operator might have changed".
   *
   * This is essentially where you need to fill in your preconditioning matrix.
   */
  virtual void setup();

  /**
   * Computes the preconditioned vector "x" based on input "y".
   * Usually by solving Px=y to get the action of P^-1 y.
   */
  virtual void apply(const NumericVector<Number> & x, NumericVector<Number> & y);

  /**
   * Release all memory and clear data structures.
   */
  virtual void clear();

protected:
  /// The nonlinear system this PC is associated with (convenience reference)
  NonlinearSystemBase & _nl;
  /// Mesh object for easy reference
  MooseMesh * _mesh;
  /// DofMap for easy reference
  DofMap * _dofmap;
  /// Number of variables
  unsigned int _n_vars;
  /// Set of dofs for each variable for every subdomain
  std::vector<std::map<SubdomainID, std::set<dof_id_type>>> _dof_sets;
  /// Set of dofs for each variable on the master
  std::vector<std::vector<dof_id_type>> _dof_sets_primary;
  /// Set of dofs on the interface
  std::vector<std::vector<dof_id_type>> _dof_sets_secondary;
  /// Set of dofs in the interior of subdomains
  std::vector<std::map<SubdomainID, std::vector<dof_id_type>>> _dof_sets_interior;

  /// Submatrices (_1 -> primary subdomain; _2 -> secondary subdomain; _i -> interior; _c -> contact interface)
  std::unique_ptr<PetscMatrix<Number>> _K2ci, _K2cc, _D, _M, _MDinv;

  std::vector<numeric_index_type> _rows, _cols;  // row and col dofs for the condensed system: rows-> all dofs except u2c; cols-> all u dofs

  /// Condensed Jacobian
  std::unique_ptr<PetscMatrix<Number>> _J_condensed;

  /// Condensed x and y
  std::unique_ptr<NumericVector<Number>> _x_hat, _y_hat, _r2c, _x2i, _x2c, _lambda;

  /// Contact
  const BoundaryID _primary_boundary, _secondary_boundary;

  /// Subdomain
  const SubdomainID _primary_subdomain, _secondary_subdomain;

  /// Whether DOFs info has been saved
  mutable bool _save_dofs;

  /// Which preconditioner to use for the solve.
  PreconditionerType _pre_type;

  /// Holds one Preconditioner object per small system to solve.
  std::unique_ptr<Preconditioner<Number>> _preconditioner;

  /// Timers
  PerfID _init_timer;
  PerfID _apply_timer;
};
