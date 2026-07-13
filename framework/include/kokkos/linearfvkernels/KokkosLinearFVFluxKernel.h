//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosLinearFVKernel.h"
#include "KokkosLinearFVBoundaryCondition.h"
#include "KokkosDatum.h"

namespace Moose::Kokkos
{

/**
 * Base class for Kokkos linear finite volume kernels that contribute on faces (flux terms). On
 * boundary faces the contribution comes from the boundary face data populated by the boundary
 * conditions acting on this kernel's variable.
 */
class LinearFVFluxKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();

  LinearFVFluxKernel(const InputParameters & parameters);
  LinearFVFluxKernel(const LinearFVFluxKernel & object);

  virtual void computeRightHandSide() override;
  virtual void computeMatrix() override;
  virtual void initialSetup() override;

  /// Whether this kernel needs boundary value data from its boundary conditions
  virtual bool needsBoundaryValueData() const { return false; }
  /// Whether this kernel needs boundary normal gradient data from its boundary conditions
  virtual bool needsBoundaryNormalGradientData() const { return false; }
  /// Whether this kernel has nonzero internal-face RHS contributions
  virtual bool hasInternalRightHandSideContribution() const { return true; }

  /// Tag dispatch type for internal-face right-hand side computation
  struct InternalRightHandSideLoop
  {
  };
  /// Tag dispatch type for boundary-face right-hand side computation
  struct BoundaryRightHandSideLoop
  {
  };
  /// Tag dispatch type for internal-face matrix computation
  struct InternalMatrixLoop
  {
  };
  /// Tag dispatch type for boundary-face matrix computation
  struct BoundaryMatrixLoop
  {
  };

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(InternalRightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(BoundaryRightHandSideLoop, const ThreadID tid, const Derived & kernel) const;

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(InternalMatrixLoop, const ThreadID tid, const Derived & kernel) const;

  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(BoundaryMatrixLoop, const ThreadID tid, const Derived & kernel) const;

  /**
   * Default methods to prevent compile errors when matrix/RHS contributions are not defined in the
   * derived class.
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real computeInternalMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeInternalNeighborMatrixContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeBoundaryMatrixContribution(
      const FVDatum & datum, const int bc_index, const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeInternalRightHandSideContribution(const FVDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real computeBoundaryRightHandSideContribution(
      const FVDatum & datum, const int bc_index, const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;
  ///@}

  /**
   * Functions used to check if users have overriden the matrix hook methods, whose calculations can
   * be skipped when not overriden.
   * @returns The function pointer of the default hook method
   */
  ///@{
  template <typename Derived>
  static auto defaultInternalMatrixContribution()
  {
    return &LinearFVFluxKernel::computeInternalMatrixContribution<Derived>;
  }

  template <typename Derived>
  static auto defaultInternalNeighborMatrixContribution()
  {
    return &LinearFVFluxKernel::computeInternalNeighborMatrixContribution<Derived>;
  }

  template <typename Derived>
  static auto defaultBoundaryMatrixContribution()
  {
    return &LinearFVFluxKernel::computeBoundaryMatrixContribution<Derived>;
  }
  ///@}

protected:
  /// Directed element side operated on by this flux kernel
  using FaceID = Pair<ContiguousElementID, unsigned int>;
  /// Directed boundary side and its compact boundary condition and BC-local face indices
  struct BoundaryFaceID
  {
    ContiguousElementID elem;
    unsigned int side;
    int bc_index;
    MOOSE_KOKKOS_INDEX_TYPE bc_face_index;

    bool operator<(const BoundaryFaceID & other) const
    {
      if (bc_index != other.bc_index)
        return bc_index < other.bc_index;
      if (elem != other.elem)
        return elem < other.elem;
      return side < other.side;
    }
  };

#ifndef NDEBUG
  /**
   * Whether this face has a mesh neighbor on a subdomain where this kernel's variable is active.
   * A mesh-internal face is treated as a boundary for a block-restricted variable when the adjacent
   * element is outside the variable's block restriction.
   */
  KOKKOS_FUNCTION bool hasFaceNeighbor(const FVDatum & datum) const;

  /// Whether this boundary face has a boundary condition data provider
  KOKKOS_FUNCTION bool hasBoundaryData(const FVDatum & datum, const int bc_index) const;
#endif

  /// Boundary value coefficient for this boundary face
  KOKKOS_FUNCTION Real boundaryValueCoefficient(const int bc_index,
                                                const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;
  /// Boundary value source for this boundary face
  KOKKOS_FUNCTION Real boundaryValueSource(const int bc_index,
                                           const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;
  /// Boundary normal gradient coefficient for this boundary face
  KOKKOS_FUNCTION Real boundaryNormalGradientCoefficient(
      const int bc_index, const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;
  /// Boundary normal gradient source for this boundary face
  KOKKOS_FUNCTION Real boundaryNormalGradientSource(
      const int bc_index, const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;

  /// Boundary condition data handles indexed by compact boundary condition index
  Array<LinearFVBoundaryCondition::BoundaryData> _bc_data;
  /// Setup/debug map from (side, element) to compact boundary condition index
  Array2D<int> _bc_index;
  /// Directed internal element sides with an active variable on the neighbor element
  Array<FaceID> _internal_face_ids;
  /// Directed boundary element sides with active Kokkos boundary data
  Array<BoundaryFaceID> _boundary_face_ids;

  std::unique_ptr<DispatcherBase> _internal_rhs_dispatcher;
  std::unique_ptr<DispatcherBase> _boundary_rhs_dispatcher;
  std::unique_ptr<DispatcherBase> _internal_matrix_dispatcher;
  std::unique_ptr<DispatcherBase> _boundary_matrix_dispatcher;
};

#ifndef NDEBUG
KOKKOS_FUNCTION inline bool
LinearFVFluxKernel::hasFaceNeighbor(const FVDatum & datum) const
{
  if (!datum.hasNeighbor())
    return false;

  const auto & sys = kokkosSystem(_var.sys());
  return sys.isVariableActive(_var.var(), datum.neighborSubdomain());
}

KOKKOS_FUNCTION inline bool
LinearFVFluxKernel::hasBoundaryData(const FVDatum & datum, const int bc_index) const
{
  return bc_index >= 0 && _bc_index.isAlloc() &&
         _bc_index(datum.side(), datum.elemID()) == bc_index;
}
#endif

KOKKOS_FUNCTION inline Real
LinearFVFluxKernel::boundaryValueCoefficient(const int bc_index,
                                             const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const
{
  KOKKOS_ASSERT(bc_index >= 0);
  return _bc_data[bc_index].value.coefficient[bc_face_index];
}

KOKKOS_FUNCTION inline Real
LinearFVFluxKernel::boundaryValueSource(const int bc_index,
                                        const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const
{
  KOKKOS_ASSERT(bc_index >= 0);
  return _bc_data[bc_index].value.source[bc_face_index];
}

KOKKOS_FUNCTION inline Real
LinearFVFluxKernel::boundaryNormalGradientCoefficient(
    const int bc_index, const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const
{
  KOKKOS_ASSERT(bc_index >= 0);
  return _bc_data[bc_index].normal_gradient.coefficient[bc_face_index];
}

KOKKOS_FUNCTION inline Real
LinearFVFluxKernel::boundaryNormalGradientSource(const int bc_index,
                                                 const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const
{
  KOKKOS_ASSERT(bc_index >= 0);
  return _bc_data[bc_index].normal_gradient.source[bc_face_index];
}

template <typename Derived>
KOKKOS_FUNCTION Real
LinearFVFluxKernel::computeInternalMatrixContribution(const FVDatum &) const
{
  ::Kokkos::abort("Default computeInternalMatrixContribution() should never be called. Make sure "
                  "you properly redefined this method in your class without typos.");

  return 0;
}

template <typename Derived>
KOKKOS_FUNCTION Real
LinearFVFluxKernel::computeInternalNeighborMatrixContribution(const FVDatum &) const
{
  ::Kokkos::abort("Default computeInternalNeighborMatrixContribution() should never be called. "
                  "Make sure you properly redefined this method in your class without typos.");

  return 0;
}

template <typename Derived>
KOKKOS_FUNCTION Real
LinearFVFluxKernel::computeBoundaryMatrixContribution(const FVDatum &,
                                                      const int,
                                                      const MOOSE_KOKKOS_INDEX_TYPE) const
{
  ::Kokkos::abort("Default computeBoundaryMatrixContribution() should never be called. Make sure "
                  "you properly redefined this method in your class without typos.");

  return 0;
}

template <typename Derived>
KOKKOS_FUNCTION Real
LinearFVFluxKernel::computeInternalRightHandSideContribution(const FVDatum &) const
{
  ::Kokkos::abort("Default computeInternalRightHandSideContribution() should never be called. Make "
                  "sure you properly redefined this method in your class without typos.");

  return 0;
}

template <typename Derived>
KOKKOS_FUNCTION Real
LinearFVFluxKernel::computeBoundaryRightHandSideContribution(const FVDatum &,
                                                             const int,
                                                             const MOOSE_KOKKOS_INDEX_TYPE) const
{
  ::Kokkos::abort("Default computeBoundaryRightHandSideContribution() should never be called. Make "
                  "sure you properly redefined this method in your class without typos.");

  return 0;
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(InternalRightHandSideLoop,
                               const ThreadID tid,
                               const Derived & kernel) const
{
  const auto [elem, side] = _internal_face_ids[tid];
  FVDatum datum(elem, side, kokkosMesh());
  KOKKOS_ASSERT(hasFaceNeighbor(datum));

  KOKKOS_ASSERT(_var.components() == 1);
  const auto & sys = kokkosSystem(_var.sys());
  kernel.accumulateTaggedVector(
      kernel.template computeInternalRightHandSideContribution<Derived>(datum),
      sys.getElemLocalDofIndex(elem, 0, _var.var()));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(BoundaryRightHandSideLoop,
                               const ThreadID tid,
                               const Derived & kernel) const
{
  const auto [elem, side, bc_index, bc_face_index] = _boundary_face_ids[tid];
  FVDatum datum(elem, side, kokkosMesh());
  KOKKOS_ASSERT(hasBoundaryData(datum, bc_index));

  KOKKOS_ASSERT(_var.components() == 1);
  const auto & sys = kokkosSystem(_var.sys());
  kernel.accumulateTaggedVector(kernel.template computeBoundaryRightHandSideContribution<Derived>(
                                    datum, bc_index, bc_face_index),
                                sys.getElemLocalDofIndex(elem, 0, _var.var()));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(InternalMatrixLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto [elem, side] = _internal_face_ids[tid];
  FVDatum datum(elem, side, kokkosMesh());
  KOKKOS_ASSERT(hasFaceNeighbor(datum));

  KOKKOS_ASSERT(_var.components() == 1);
  const auto var_num = _var.var();
  const auto & sys = kokkosSystem(_var.sys());
  const auto row = sys.getElemLocalDofIndex(elem, 0, var_num);
  kernel.accumulateTaggedMatrix(kernel.template computeInternalMatrixContribution<Derived>(datum),
                                row,
                                sys.getElemGlobalDofIndex(elem, 0, var_num));
  kernel.accumulateTaggedMatrix(
      kernel.template computeInternalNeighborMatrixContribution<Derived>(datum),
      row,
      sys.getElemGlobalDofIndex(datum.neighborID(), 0, var_num));
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVFluxKernel::operator()(BoundaryMatrixLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto [elem, side, bc_index, bc_face_index] = _boundary_face_ids[tid];
  FVDatum datum(elem, side, kokkosMesh());
  KOKKOS_ASSERT(hasBoundaryData(datum, bc_index));

  KOKKOS_ASSERT(_var.components() == 1);
  const auto var_num = _var.var();
  const auto & sys = kokkosSystem(_var.sys());
  const auto row = sys.getElemLocalDofIndex(elem, 0, var_num);
  kernel.accumulateTaggedMatrix(
      kernel.template computeBoundaryMatrixContribution<Derived>(datum, bc_index, bc_face_index),
      row,
      sys.getElemGlobalDofIndex(elem, 0, var_num));
}

} // namespace Moose::Kokkos
