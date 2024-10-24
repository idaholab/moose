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

// C++ includes
#include <vector>

// Forward declarations
class NonlinearSystemBase;
/**
 * Implements a segregated solve preconditioner.
 */
class PhysicsBasedPreconditioner : public MoosePreconditioner,
                                   public libMesh::Preconditioner<Number>
{
public:
  /**
   *  Constructor. Initializes PhysicsBasedPreconditioner data structures
   */
  static InputParameters validParams();

  PhysicsBasedPreconditioner(const InputParameters & params);
  virtual ~PhysicsBasedPreconditioner();

  /**
   * Add a diagonal system + possibly off-diagonals ones as well, also specifying type of
   * preconditioning
   */
  // FIXME: use better name
  void addSystem(unsigned int var,
                 std::vector<unsigned int> off_diag,
                 libMesh::PreconditionerType type = libMesh::AMG_PRECOND);

  /**
   * Computes the preconditioned vector "y" based on input "x".
   * Usually by solving Py=x to get the action of P^-1 x.
   */
  virtual void apply(const NumericVector<Number> & x, NumericVector<Number> & y);

  /**
   * Release all memory and clear data structures.
   */
  virtual void clear();

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

protected:
  /// The nonlinear system this PBP is associated with (convenience reference)
  NonlinearSystemBase & _nl;
  /// List of linear system that build up the preconditioner
  std::vector<libMesh::LinearImplicitSystem *> _systems;
  /// Holds one Preconditioner object per small system to solve.
  std::vector<std::unique_ptr<libMesh::Preconditioner<Number>>> _preconditioners;
  /// Holds the order the blocks are solved for.
  std::vector<unsigned int> _solve_order;
  /// Which preconditioner to use for each solve.
  std::vector<libMesh::PreconditionerType> _pre_type;
  /// Holds which off diagonal blocks to compute.
  std::vector<std::vector<unsigned int>> _off_diag;

  /**
   * Holds pointers to the off-diagonal matrices.
   * This is in the same order as _off_diag.
   *
   * This is really just for convenience so we don't have
   * to keep looking this thing up through it's name.
   */
  std::vector<std::vector<libMesh::SparseMatrix<Number> *>> _off_diag_mats;
};
