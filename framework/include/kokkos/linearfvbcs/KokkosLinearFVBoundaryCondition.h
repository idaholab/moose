//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDispatcher.h"
#include "KokkosDatum.h"
#include "KokkosMesh.h"
#include "KokkosSystem.h"
#include "KokkosVariable.h"

#include "BoundaryRestrictableRequired.h"
#include "FunctionInterface.h"
#include "MeshChangedInterface.h"
#include "MooseObject.h"
#include "PostprocessorInterface.h"
#include "Restartable.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"
#include "VectorPostprocessorInterface.h"

class FEProblemBase;
class InputParameters;

namespace Moose::Kokkos
{

/**
 * Base class for Kokkos linear finite volume boundary conditions. Boundary conditions provide
 * boundary data relations for flux kernels; they do not assemble directly into tagged vectors or
 * matrices.
 */
class LinearFVBoundaryCondition : public MooseObject,
                                  public SetupInterface,
                                  public FunctionInterface,
                                  public UserObjectInterface,
                                  public TransientInterface,
                                  public PostprocessorInterface,
                                  public VectorPostprocessorInterface,
                                  public Restartable,
                                  public MeshChangedInterface,
                                  public BoundaryRestrictableRequired,
                                  public MeshHolder,
                                  public SystemHolder
{
public:
  static InputParameters validParams();

  LinearFVBoundaryCondition(const InputParameters & parameters);
  LinearFVBoundaryCondition(const LinearFVBoundaryCondition & object);

  /// Tag dispatch type for boundary value relation computation
  struct BoundaryValueLoop
  {
  };
  /// Tag dispatch type for boundary normal gradient relation computation
  struct BoundaryNormalGradientLoop
  {
  };

  /**
   * Affine boundary relation used by Kokkos linear FV kernels:
   *   boundary_quantity = coefficient * cell_unknown + source
   */
  struct BoundaryRelation
  {
    Real coefficient = 0;
    Real source = 0;
  };

  /// Boundary relation coefficient/source arrays indexed by BC-local boundary face
  struct BoundaryRelationData
  {
    Array<Real> coefficient;
    Array<Real> source;
  };

  /// Boundary relation data indexed by relation type and BC-local boundary face
  struct BoundaryData
  {
    BoundaryRelationData value;
    BoundaryRelationData normal_gradient;
  };

  virtual void initialSetup() override;

  /**
   * Get the Kokkos variable this boundary condition supplies data for
   * @returns The Kokkos variable
   */
  Variable variable() const { return _var; }

  /**
   * Get the boundary data relation arrays owned by this boundary condition
   */
  const BoundaryData & boundaryData() const { return _boundary_data; }

  /// Number of faces in this boundary condition's existing worklist
  MOOSE_KOKKOS_INDEX_TYPE numBoundaryFaces() const;

  /// Face at a BC-local worklist index
  Pair<ContiguousElementID, unsigned int>
  boundaryFaceID(const MOOSE_KOKKOS_INDEX_TYPE bc_face_index) const;

  /// Whether this boundary condition overrides the boundary value relation hook
  bool hasBoundaryValue() const;
  /// Whether this boundary condition overrides the boundary normal gradient relation hook
  bool hasBoundaryNormalGradient() const;

  /// Dispatch boundary value relation computation
  void computeBoundaryValueData();
  /// Dispatch boundary normal gradient relation computation
  void computeBoundaryNormalGradientData();

  /**
   * Default boundary value relation hook. Derived boundary conditions should override this when
   * they can supply boundary values.
   */
  template <typename Derived>
  KOKKOS_FUNCTION BoundaryRelation computeBoundaryValue(const FVDatum &) const
  {
    ::Kokkos::abort("Default computeBoundaryValue() should never be called. Make sure you "
                    "properly redefined this method in your class without typos.");

    return {};
  }

  /**
   * Default boundary normal gradient relation hook. Derived boundary conditions should override
   * this when they can supply boundary normal gradients.
   */
  template <typename Derived>
  KOKKOS_FUNCTION BoundaryRelation computeBoundaryNormalGradient(const FVDatum &) const
  {
    ::Kokkos::abort("Default computeBoundaryNormalGradient() should never be called. Make sure "
                    "you properly redefined this method in your class without typos.");

    return {};
  }

  /**
   * Functions used to check whether derived boundary conditions override the boundary data hooks.
   */
  ///@{
  template <typename Derived>
  static auto defaultBoundaryValue()
  {
    return &LinearFVBoundaryCondition::computeBoundaryValue<Derived>;
  }

  template <typename Derived>
  static auto defaultBoundaryNormalGradient()
  {
    return &LinearFVBoundaryCondition::computeBoundaryNormalGradient<Derived>;
  }
  ///@}

  /**
   * Boundary value dispatch loop body; writes the per-face boundary value relation
   * @param tid The thread ID of the current boundary side
   * @param bc The concrete boundary condition object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(BoundaryValueLoop, const ThreadID tid, const Derived & bc) const;

  /**
   * Boundary normal gradient dispatch loop body; writes the per-face boundary normal gradient
   * relation
   * @param tid The thread ID of the current boundary side
   * @param bc The concrete boundary condition object
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(BoundaryNormalGradientLoop, const ThreadID tid, const Derived & bc) const;

protected:
  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

  /// Kokkos variable
  Variable _var;

  /// Boundary relation arrays owned by this boundary condition
  BoundaryData _boundary_data;

  /// Dispatcher for boundary value computation
  std::unique_ptr<DispatcherBase> _boundary_value_dispatcher;
  /// Dispatcher for boundary normal gradient computation
  std::unique_ptr<DispatcherBase> _boundary_normal_gradient_dispatcher;

  /**
   * TODO: Move to TransientInterface
   */
  ///@{
  /// Current time
  Scalar<Real> _t;
  /// Old (previous time step) time
  Scalar<const Real> _t_old;
  /// Current time step number
  Scalar<int> _t_step;
  /// Current time step size
  Scalar<Real> _dt;
  /// Previous time step size
  Scalar<Real> _dt_old;
  ///@}
};

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVBoundaryCondition::operator()(BoundaryValueLoop,
                                      const ThreadID tid,
                                      const Derived & bc) const
{
  const auto [elem, side] = kokkosBoundaryElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  const auto relation = bc.template computeBoundaryValue<Derived>(datum);
  _boundary_data.value.coefficient[tid] = relation.coefficient;
  _boundary_data.value.source[tid] = relation.source;
}

template <typename Derived>
KOKKOS_FUNCTION void
LinearFVBoundaryCondition::operator()(BoundaryNormalGradientLoop,
                                      const ThreadID tid,
                                      const Derived & bc) const
{
  const auto [elem, side] = kokkosBoundaryElementSideID(tid);
  FVDatum datum(elem, side, kokkosMesh());
  const auto relation = bc.template computeBoundaryNormalGradient<Derived>(datum);
  _boundary_data.normal_gradient.coefficient[tid] = relation.coefficient;
  _boundary_data.normal_gradient.source[tid] = relation.source;
}

} // namespace Moose::Kokkos
