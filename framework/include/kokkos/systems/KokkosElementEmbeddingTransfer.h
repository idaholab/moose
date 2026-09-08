//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLevelTransfer.h"

#ifdef MOOSE_KOKKOS_SCOPE

#include "KokkosVector.h"
#include "KokkosThread.h"
#include "KokkosAssembly.h"

namespace Moose::Kokkos
{

/**
 * Slots of the vector array a per-element embedding gathers from and scatters into
 */
enum EmbeddingVectorSlot : TagID
{
  /// The vector on the coarse side of the level pair
  EMBEDDING_COARSE = 0,
  /// The vector on the fine side of the level pair
  EMBEDDING_FINE = 1,
  EMBEDDING_NUM_SLOTS = 2
};

/**
 * The device loops of a per-element embedding: gather one element's coarse DOFs, apply the
 * reference-element embedding, and scatter-add onto the element's fine DOFs, along with the
 * transpose of that and the count of elements each fine DOF belongs to.
 *
 * A loop functor is copied to device, so this holds the two sides by value, and the transfer that
 * owns the embedding tables and the work vectors builds one of these per dispatch.
 */
class ElementEmbedding
{
public:
  /**
   * Constructor
   * @param coarse The DOF layout of the coarse side
   * @param fine The DOF layout of the fine side
   * @param embedding The reference-element embedding of the coarse basis in the fine basis, indexed
   * by (subdomain, element type, variable, orientation) and sized (fine DOFs, coarse DOFs)
   * @param vectors The vectors of the two sides, indexed by EmbeddingVectorSlot
   * @param coarse_constrained Local-plus-ghost mask of the coarse side's fixed DOFs, which may be
   * unallocated when the coarse side holds none fixed
   * @param fine_constrained Local-plus-ghost mask of the fine side's fixed DOFs, which may be
   * unallocated when the fine side holds none fixed
   */
  ElementEmbedding(const DofSpace & coarse,
                   const DofSpace & fine,
                   const Array4D<Array2D<Real>> & embedding,
                   const Array<Vector> & vectors,
                   const Array<bool> & coarse_constrained,
                   const Array<bool> & fine_constrained);

  /**
   * Kokkos function tags for the loops over (element, variable) pairs
   */
  ///@{
  struct ProlongLoop
  {
  };
  struct RestrictLoop
  {
  };
  struct MultiplicityLoop
  {
  };
  ///@}

  /**
   * Accumulate the embedding of the coarse vector onto the fine vector
   */
  void prolong();

  /**
   * Accumulate the transpose of the embedding of the fine vector onto the coarse vector
   */
  void restrict();

  /**
   * Accumulate the number of elements each fine DOF belongs to onto the fine vector
   */
  void multiplicity();

  KOKKOS_FUNCTION void operator()(ProlongLoop, const ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(RestrictLoop, const ThreadID tid) const;
  KOKKOS_FUNCTION void operator()(MultiplicityLoop, const ThreadID tid) const;

private:
  /**
   * Get the embedding table of an (element, variable) pair
   * @param elem The contiguous element ID
   * @param var The variable number
   * @returns The embedding table, which is unallocated when the variable has no DOFs to transfer on
   * the element
   */
  KOKKOS_FUNCTION const Array2D<Real> & table(ContiguousElementID elem, unsigned int var) const;

  /**
   * Get whether the coarse side holds a degree of freedom fixed, which keeps it out of both
   * directions of the transfer
   * @param dof The local DOF index on the coarse side
   * @returns Whether the coarse side holds the DOF fixed
   */
  KOKKOS_FUNCTION bool coarseConstrained(const dof_id_type dof) const
  {
    return _coarse_constrained.isAlloc() && _coarse_constrained[dof];
  }

  /**
   * Get whether the fine side holds a degree of freedom fixed, which keeps it out of both
   * directions of the transfer
   * @param dof The local DOF index on the fine side
   * @returns Whether the fine side holds the DOF fixed
   */
  KOKKOS_FUNCTION bool fineConstrained(const dof_id_type dof) const
  {
    return _fine_constrained.isAlloc() && _fine_constrained[dof];
  }

  /// The DOF layout of the coarse side
  const DofSpace _coarse;

  /// The DOF layout of the fine side
  const DofSpace _fine;

  /// The reference-element embedding, indexed by (subdomain, element type, variable, orientation)
  const Array4D<Array2D<Real>> _embedding;

  /// The vectors of the two sides, indexed by EmbeddingVectorSlot
  const Array<Vector> _vectors;

  /// Local-plus-ghost mask of the coarse side's fixed DOFs
  const Array<bool> _coarse_constrained;

  /// Local-plus-ghost mask of the fine side's fixed DOFs
  const Array<bool> _fine_constrained;

  /// Kokkos thread object over the (element, variable) pairs of the loops
  Thread<> _thread;
};

/**
 * Transfer between two levels of a p-multigrid hierarchy whose spaces are nested as functions while
 * sharing no degrees of freedom, which is the case for Lagrange p-coarsening on a fixed mesh.
 *
 * Per (subdomain, element type, variable) a small dense reference-element matrix expresses each
 * coarse basis function in the fine basis. That matrix depends on the reference element alone, so
 * it is built once on the host at setup, while an application is the device-side gather of an
 * element's coarse DOFs, the dense multiply, and the scatter-add onto the element's fine DOFs.
 *
 * An element scatters the fine DOF values of its own coarse DOFs, so a fine DOF shared by several
 * elements receives the same value once per element it belongs to; dividing by that count recovers
 * the prolongation. Restriction applies the same count to the fine vector ahead of the transpose
 * gather, which makes it exactly the transpose of prolongation.
 */
class ElementEmbeddingTransfer : public LevelTransfer
{
public:
  /**
   * Constructor. Builds the reference-element embedding of the coarse basis in the fine basis and
   * the count of elements each fine DOF belongs to, and errors out if the coarse space of any
   * variable is not contained in the fine space, in which case an embedding does not exist.
   * @param assembly The Kokkos assembly holding the cached reference basis tables
   * @param coarse The coarse side of the level pair
   * @param fine The fine side of the level pair
   */
  ElementEmbeddingTransfer(const Assembly & assembly,
                           const TransferSide & coarse,
                           const TransferSide & fine);

  virtual void prolong(const libMesh::NumericVector<Number> & x,
                       libMesh::NumericVector<Number> & y) override;

  virtual void restrict(const libMesh::NumericVector<Number> & x,
                        libMesh::NumericVector<Number> & y) override;

private:
  /**
   * Build the reference-element embedding of the coarse basis in the fine basis for every
   * (subdomain, element type, variable), and check that it reproduces the coarse basis
   */
  void buildEmbedding();

  /**
   * Build the reciprocal of the number of elements each fine DOF belongs to, which is the weighting
   * an element-wise scatter of shared DOF values needs
   */
  void buildMultiplicity();

  /**
   * Build the loop functor over the two sides
   * @returns The functor
   */
  ElementEmbedding loops() const;

  /// The Kokkos assembly holding the cached reference basis tables
  const Assembly & _assembly;

  /// The reference-element embedding, indexed by (subdomain, element type, variable, orientation)
  Array4D<Array2D<Real>> _embedding;

  /// The vectors of the two sides, indexed by EmbeddingVectorSlot
  Array<Vector> _vectors;

  /// The coarse side's ghosted work vector, which carries the input of a prolongation
  libMesh::NumericVector<Number> & _coarse_input;

  /// The fine side's ghosted work vector, which carries the input of a restriction
  libMesh::NumericVector<Number> & _fine_input;

  /// The reciprocal of the number of elements each fine DOF belongs to
  libMesh::NumericVector<Number> & _inverse_multiplicity;
};

} // namespace Moose::Kokkos

#endif
