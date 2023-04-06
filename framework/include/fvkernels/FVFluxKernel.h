//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVKernel.h"
#include "MathFVUtils.h"
#include "NeighborCoupleable.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborMooseVariableInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FVFaceResidualObject.h"
#include "FaceArgInterface.h"

class FaceInfo;

/// FVFluxKernel is used for calculating residual contributions from numerical
/// fluxes from surface integral terms in a finite volume discretization of a
/// PDE (i.e.  terms where the divergence theorem is applied).  As with finite
/// element kernels, all solution values and material properties must be
/// indexed using the _qp member.  Note that all interfaces for finite volume
/// kernels are AD-based - be sure to use AD material properties and other AD
/// values to maintain good jacobian/derivative quality.
class FVFluxKernel : public FVKernel,
                     public TwoMaterialPropertyInterface,
                     public NeighborMooseVariableInterface<Real>,
                     public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                     public FVFaceResidualObject,
                     public FaceArgProducerInterface
{
public:
  static InputParameters validParams();
  FVFluxKernel(const InputParameters & params);

  void computeResidual() override;
  void computeJacobian() override;
  void computeResidualAndJacobian() override;
  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;
  void computeResidualAndJacobian(const FaceInfo & fi) override;

  const MooseVariableFV<Real> & variable() const override { return _var; }

  bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

protected:
  /// This is the primary function that must be implemented for flux kernel
  /// terms.  Material properties will be initialized on the face - using any
  /// reconstructed fv variable gradients if any.  Values for the solution are
  /// provided for both the elem and neighbor side of the face.
  virtual ADReal computeQpResidual() = 0;

  /// Calculates and returns "grad_u dot normal" on the face to be used for
  /// diffusive terms.  If using any cross-diffusion corrections, etc. all
  /// those calculations will be handled for appropriately by this function.
  virtual ADReal gradUDotNormal(const Moose::StateArg & time) const;

  /// Kernels are called even on boundaries in case one is for a variable with
  /// a dirichlet BC - in which case we need to run the kernel with a
  /// ghost-element.  This returns true if we need to run because of dirichlet
  /// conditions - otherwise this returns false and all jacobian/residual calcs
  /// should be skipped.
  virtual bool skipForBoundary(const FaceInfo & fi) const;

  const RealVectorValue & normal() const { return _normal; }

  MooseVariableFV<Real> & _var;

  const unsigned int _qp = 0;

  /// The elem solution value of the kernel's _var for the current face.
  const ADVariableValue & _u_elem;
  /// The neighbor solution value of the kernel's _var for the current face.
  const ADVariableValue & _u_neighbor;

  /// This is the outward unit normal vector for the face the kernel is currently
  /// operating on.  By convention, this is set to be pointing outward from the
  /// face's elem element and residual calculations should keep this in mind.
  RealVectorValue _normal;

  /// This is holds meta-data for geometric information relevant to the current
  /// face including elem+neighbor cell centroids, cell volumes, face area, etc.
  const FaceInfo * _face_info = nullptr;

  /// The face type
  FaceInfo::VarFaceNeighbors _face_type;

  /**
   * Return whether the supplied face is on a boundary of this object's execution
   */
  bool onBoundary(const FaceInfo & fi) const;

  /**
   * @return an element argument corresponding to the face info elem
   */
  Moose::ElemArg elemArg(bool correct_skewness = false) const;

  /**
   * @return an element argument corresponding to the face info neighbor
   */
  Moose::ElemArg neighborArg(bool correct_skewness = false) const;

  /**
   * Determine the single sided face argument when evaluating a functor on a face.
   * This is used to perform evaluations of material properties with the actual face values of
   * their dependences, rather than interpolate the material property to the boundary.
   * @param fi the FaceInfo for this face
   * @param limiter_type the limiter type, to be specified if more than the default average
   *        interpolation is required for the parameters of the functor
   * @param correct_skewness whether to perform skew correction at the face
   */
  Moose::FaceArg singleSidedFaceArg(
      const FaceInfo * fi = nullptr,
      Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
      bool correct_skewness = false) const;

  /**
   * Returns whether to avoid execution on a boundary
   * @param fi the FaceInformation currently considered
   */
  bool avoidBoundary(const FaceInfo & fi) const;

  /**
   * Adjust the number of ghost layers in the relationship manager
   * @param ghost_layers The new number of requested ghost layers
   */
  void adjustRMGhostLayers(const unsigned short ghost_layers) const;

  /// Which boundaries/sidesets to force the execution of flux kernels on
  std::unordered_set<BoundaryID> _boundaries_to_force;

private:
  /// Computes the Jacobian contribution for every coupled variable.
  ///
  /// @param type Either ElementElement, ElementNeighbor, NeighborElement, or NeighborNeighbor. As an
  /// example ElementNeighbor means the derivatives of the elemental residual with respect to the
  /// neighbor degrees of freedom.
  ///
  /// @param residual The already computed residual (probably done with \p computeQpResidual) that
  /// also holds derivative information for filling in the Jacobians.
  void computeJacobianType(Moose::DGJacobianType type, const ADReal & residual);

  /// Whether to force execution of flux kernels on all external boundaries
  const bool _force_boundary_execution;

  /// Which boundaries/sidesets to prevent the execution of flux kernels on
  std::unordered_set<BoundaryID> _boundaries_to_avoid;
};
