//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDofSpace.h"
#include "KokkosVector.h"

#include "libmesh/system.h"

class NonlinearSystemBase;

namespace Moose::Kokkos
{

class QpJacobianCache;
class QpJacobianLevel;

/**
 * One coarse level of a p-multigrid hierarchy: the fine system's variables at a reduced polynomial
 * order, on the same mesh.
 *
 * A level carries no residual objects. Its linearization is the quadrature-point Jacobian cache the
 * fine level fills, contracted against this level's basis tables, so all a level needs to own is a
 * function space: a libMesh system supplies the DofMap, the parallel ghosting, and the vectors,
 * while the reference basis tables come from the Kokkos assembly, which caches them per (subdomain,
 * element type, FE type) at the fine quadrature rule.
 *
 * The level system must be added before the equation systems are initialized, so a level space is
 * constructed at preconditioner-construction time and its DOF layout, which needs distributed DOFs,
 * is built later by init().
 */
class PLevelSpace
{
public:
  /**
   * Constructor. Adds the level's libMesh system and its variables, and registers the system with
   * the Kokkos assembly so that reference shape data is cached for the level's FE types.
   * @param fine The fine solver system whose variables the level reproduces at reduced order
   * @param order The polynomial order of the level
   */
  PLevelSpace(NonlinearSystemBase & fine, unsigned int order);

  /**
   * Build the level's device DOF layout. Must be called once the equation systems are initialized
   * and the Kokkos mesh is available, which is to say no earlier than initial setup.
   */
  void init();

  /**
   * Get the polynomial order of the level
   * @returns The order
   */
  unsigned int order() const { return _order; }

  /**
   * Apply the level's operator, which is the fine level's quadrature-point Jacobian cache
   * contracted against this level's basis tables
   * @param cache The quadrature-point Jacobian cache, holding a linearization
   * @param x The vector to apply the operator to, in the level's DOF layout
   * @param y The result, in the level's DOF layout
   */
  void apply(const QpJacobianCache & cache,
             const libMesh::NumericVector<Number> & x,
             libMesh::NumericVector<Number> & y);

  /**
   * Compute the diagonal of the level's operator, which a Jacobi or Chebyshev smoother reads
   * @param cache The quadrature-point Jacobian cache, holding a linearization
   * @param diagonal The diagonal, in the level's DOF layout
   */
  void diagonal(const QpJacobianCache & cache, libMesh::NumericVector<Number> & diagonal);

  /**
   * Get the level's device DOF layout
   * @returns The DOF layout
   */
  const DofSpace & dofSpace() const;

  /**
   * Get the level's libMesh system
   * @returns The system
   */
  ///@{
  libMesh::System & system() { return _sys; }
  const libMesh::System & system() const { return _sys; }
  ///@}

  /**
   * Get the name of the libMesh system a level of a given order gets
   * @param fine The fine solver system
   * @param order The polynomial order of the level
   * @returns The system name
   */
  static std::string systemName(const NonlinearSystemBase & fine, unsigned int order);

private:
  /**
   * Slots of the level's vector array, which is what the level context the operator runs against
   * indexes
   */
  enum VectorSlot : TagID
  {
    /// The vector the operator is applied to
    X = 0,
    /// The vector the operator scatters into
    Y = 1,
    NUM_SLOTS = 2
  };

  /**
   * Build the level context the operator runs against: this level's DOF layout, FE types and
   * vectors
   * @returns The level context
   */
  QpJacobianLevel level() const;

  /**
   * Wrap the level's vectors for device access and dispatch the operator
   * @param cache The quadrature-point Jacobian cache, holding a linearization
   * @param y The vector the operator scatters into
   * @param x The vector the operator is applied to, or null for the diagonal, which reads none
   */
  void dispatch(const QpJacobianCache & cache,
                libMesh::NumericVector<Number> & y,
                const libMesh::NumericVector<Number> * x);

  /// The fine solver system whose variables the level reproduces at reduced order
  NonlinearSystemBase & _fine;

  /// The polynomial order of the level
  const unsigned int _order;

  /// The level's libMesh system, which owns its DofMap and vectors
  libMesh::System & _sys;

  /// The level's device DOF layout, built by init()
  std::unique_ptr<DofSpace> _dof_space;

  /// The FE type ID of each variable, as the Kokkos assembly indexes its reference shape data
  Array<unsigned int> _fe_types;

  /// The level's vectors, indexed by VectorSlot
  Array<Vector> _vectors;

  /// The level's ghosted work vector, which carries the input of an operator application
  libMesh::NumericVector<Number> * _x = nullptr;
};

} // namespace Moose::Kokkos
