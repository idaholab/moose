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
#include "KokkosEntityBlocks.h"
#include "KokkosMatrix.h"
#include "KokkosVector.h"

#include "libmesh/system.h"
#include "libmesh/wrapped_petsc.h"

class ConsoleStream;
class NonlinearSystemBase;

namespace Moose::Kokkos
{

class LevelTransfer;
class QpJacobianCache;
class QpJacobianLevel;
struct TransferSide;

/**
 * One coarse level of a p-multigrid hierarchy: the fine system's variables at a reduced polynomial
 * order, on the same mesh.
 *
 * A level carries no residual objects. Its linearization is the quadrature-point Jacobian cache the
 * fine level fills, contracted against this level's basis tables, so all a level needs to own is a
 * function space: a libMesh system supplies the DofMap, the parallel ghosting, and the vectors,
 * while the reference basis tables come from the Kokkos assembly, which caches them per (subdomain,
 * element type, FE type, edge and face orientation) at the fine quadrature rule.
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
   * @param assemble Whether the level assembles its operator into a sparse matrix, which the
   * coarsest level does so that a coarse solver can be applied to it
   */
  PLevelSpace(NonlinearSystemBase & fine, unsigned int order, bool assemble = false);

  /**
   * Destructor
   */
  ~PLevelSpace();

  /**
   * Build the level's device DOF layout. Must be called once the equation systems are initialized
   * and the Kokkos mesh is available, which is to say no earlier than initial setup.
   */
  void init();

  /**
   * Build the transfer between this level and the next finer level. Must be called once init() has
   * been called on both levels, so that both sides have a DOF layout.
   * @param finer The next finer level, or null when the next finer level is the solver system
   */
  void initTransfer(PLevelSpace * finer);

  /**
   * Check that the transfer to the next finer level restricts by the transpose of the way it
   * prolongs, and report the agreement
   * @param console The stream the agreement is reported on
   */
  void verifyTransfer(const ConsoleStream & console);

  /**
   * Get the polynomial order of the level
   * @returns The order
   */
  unsigned int order() const { return _order; }

  /**
   * Get the number of the level's degrees of freedom that a nodal boundary condition of the fine
   * system pins, over all processes
   * @returns The number of constrained DOFs
   */
  dof_id_type numConstrainedDofs() const { return _num_constrained_dofs; }

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
   * Assemble the level's operator into the level's sparse matrix, which is the same contraction the
   * operator action performs, taken entry by entry. Only a level constructed to assemble has a
   * matrix to fill.
   * @param cache The quadrature-point Jacobian cache, holding a linearization
   */
  void assembleMatrix(const QpJacobianCache & cache);

  /**
   * Get whether the level assembles its operator into a sparse matrix
   * @returns Whether the level assembles
   */
  bool assembles() const { return _assemble; }

  /**
   * Get the level's assembled operator, which a coarse solver is applied to
   * @returns The matrix
   */
  libMesh::SparseMatrix<Number> & matrix();

  /**
   * Bring the level's operator up to date with the linearization the quadrature-point Jacobian
   * cache holds. A level that assembles refills its matrix; every other level marks its shell
   * changed, which is what makes the level's smoother rebuild what it reads from the operator.
   * @param cache The quadrature-point Jacobian cache, holding a linearization
   */
  void updateOperator(const QpJacobianCache & cache);

  /**
   * Get the level's operator as a PETSc matrix, which is what a level of PETSc's multigrid takes
   * for both its Krylov operator and its smoother's preconditioning matrix. A level that assembles
   * hands over its assembled matrix; every other level hands over a shell whose matrix-vector
   * product and diagonal are this level's contraction of the quadrature-point Jacobian cache.
   * @returns The operator
   */
  Mat operatorMat() const;

  /**
   * Get the transfer to the next finer level as a PETSc matrix, whose product prolongs and whose
   * transposed product restricts, which is what PETSc's multigrid takes for its interpolation
   * @returns The interpolation
   */
  Mat interpolationMat() const;

  /**
   * Entry points of the level's shell matrices, which read the linearization from the fine system's
   * quadrature-point Jacobian cache
   */
  ///@{
  /// The operator shell's matrix-vector product
  void applyToVec(Vec x, Vec y);
  /// The operator shell's diagonal
  void diagonalToVec(Vec diagonal);
  /// The interpolation shell's matrix-vector product
  void prolongToVec(Vec x, Vec y);
  /// The interpolation shell's transposed matrix-vector product
  void restrictToVec(Vec x, Vec y);
  ///@}

  /**
   * Prolong a vector of this level to the next finer level, y = P x
   * @param x The vector on this level
   * @param y The vector on the next finer level
   */
  void prolong(const libMesh::NumericVector<Number> & x, libMesh::NumericVector<Number> & y);

  /**
   * Restrict a vector of the next finer level to this level, y = P^T x
   * @param x The vector on the next finer level
   * @param y The vector on this level
   */
  void restrict(const libMesh::NumericVector<Number> & x, libMesh::NumericVector<Number> & y);

  /**
   * Zero the entries of a vector of this level that the level holds fixed, which leaves the vector
   * in the space of corrections the hierarchy carries
   * @param x The vector to zero the fixed entries of
   */
  void zeroConstrained(libMesh::NumericVector<Number> & x);

  /**
   * Build the level's entity-block decomposition, which its smoother inverts in place of the
   * operator diagonal. Must be called once the level's DOF layout exists.
   */
  void initEntityBlocks();

  /**
   * Get the number of entity blocks the level carries, over all processes
   * @returns The number of blocks
   */
  dof_id_type numEntityBlocks() const { return _num_entity_blocks; }

  /**
   * Get the size of the largest entity block the level carries, over all processes
   * @returns The largest block size
   */
  unsigned int maxEntityBlockSize() const { return _max_entity_block_size; }

  /**
   * Install the level's entity-block smoother on a preconditioner, as a shell whose setup rebuilds
   * the blocks from the current linearization and whose application inverts them
   * @param pc The preconditioner of the level's smoother
   */
  void setupBlockSmootherPC(PC pc);

  /**
   * Entry points of the level's entity-block smoother shell
   */
  ///@{
  /// Rebuild and refactor the blocks from the linearization the cache holds
  void setupBlockSmoother();
  /// Apply the inverse of every block to a residual
  void applyBlockSmoother(Vec r, Vec x);
  ///@}

  /**
   * Get the level's device DOF layout
   * @returns The DOF layout
   */
  const DofSpace & dofSpace() const;

  /**
   * Get this level as one side of a level pair, which is what a transfer between levels indexes
   * @returns The side
   */
  TransferSide transferSide() const;

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
   * Build the mask of the level's degrees of freedom that a nodal boundary condition of the fine
   * system pins, along with the vector that carries the identity on those rows
   */
  void setupConstrainedDofs();

  /**
   * Build the level context the operator runs against: this level's DOF layout, FE types and
   * vectors
   * @returns The level context
   */
  QpJacobianLevel level() const;

  /**
   * Get the fine system's quadrature-point Jacobian cache, which every level's operator contracts
   * @returns The cache
   */
  const QpJacobianCache & cache() const;

  /**
   * Create a shell matrix over the level's DOF layout
   * @param rows The system whose DOF layout the shell's rows are over
   * @param columns The system whose DOF layout the shell's columns are over
   * @param mat The shell
   */
  void createShell(const libMesh::System & rows, const libMesh::System & columns, Mat & mat);

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

  /// Whether the level assembles its operator into a sparse matrix
  const bool _assemble;

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

  /**
   * Local-plus-ghost mask of the level's DOFs that a nodal boundary condition of the fine system
   * pins, which the level's operator holds fixed
   */
  Array<bool> _constrained_dof;

  /// One on the DOFs the level holds fixed and zero elsewhere, which places the identity on the
  /// operator's fixed rows
  libMesh::NumericVector<Number> * _constrained_rows = nullptr;

  /// The number of DOFs the level holds fixed, over all processes
  dof_id_type _num_constrained_dofs = 0;

  /// The level's entity-block decomposition, built only when its smoother inverts the blocks
  EntityBlocks _blocks;

  /// The number of entity blocks the level carries, over all processes
  dof_id_type _num_entity_blocks = 0;

  /// The size of the largest entity block the level carries, over all processes
  unsigned int _max_entity_block_size = 0;

  /// The level's assembled operator, which only a level that assembles carries
  libMesh::SparseMatrix<Number> * _matrix_object = nullptr;

  /// The level's assembled operator wrapped for device assembly
  Matrix _matrix;

  /// The level's operator as a shell matrix, which a level that assembles its operator instead of
  /// applying it does not carry
  libMesh::WrappedPetsc<Mat> _operator;

  /// The transfer to the next finer level as a shell matrix, built by initTransfer()
  libMesh::WrappedPetsc<Mat> _interpolation;

  /// The transfer between this level and the next finer level, built by initTransfer()
  std::unique_ptr<LevelTransfer> _prolongation;
};

} // namespace Moose::Kokkos
