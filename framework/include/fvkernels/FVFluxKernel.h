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
                     public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();
  FVFluxKernel(const InputParameters & params);

  /// Usually you should not override these functions - they have some super
  /// tricky stuff in them that you don't want to mess up!
  // @{
  virtual void computeResidual(const FaceInfo & fi);
  virtual void computeJacobian(const FaceInfo & fi);
  /// @}

  const MooseVariableFV<Real> & variable() const { return _var; }

protected:
  /// This is the primary function that must be implemented for flux kernel
  /// terms.  Material properties will be initialized on the face - using any
  /// reconstructed fv variable gradients if any.  Values for the solution are
  /// provided for both the elem and neighbor side of the face.
  virtual ADReal computeQpResidual() = 0;

  /// Calculates and returns "grad_u dot normal" on the face to be used for
  /// diffusive terms.  If using any cross-diffusion corrections, etc. all
  /// those calculations will be handled for appropriately by this function.
  virtual ADReal gradUDotNormal() const;

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
   * @return the value of \p makeSidedFace called with the face info element
   */
  Moose::ElemFromFaceArg elemFromFace(bool correct_skewness = false) const;

  /**
   * @return the value of \p makeSidedFace called with the face info neighbor
   */
  Moose::ElemFromFaceArg neighborFromFace(bool correct_skewness = false) const;

  /**
   * Determine the subdomain ID pair that should be used when creating a face argument for a
   * functor. The first member of the pair will correspond to the SubdomainID in the tuple returned
   * by \p elemFromFace. The second member of the pair will correspond to the SubdomainID in the
   * tuple returned by \p neighborFromFace. As explained in the doxygen for \p makeSidedFace these
   * subdomain IDs do not simply correspond to the subdomain ID of the element; they must respect
   * the block restriction of this object
   */
  std::pair<SubdomainID, SubdomainID> faceArgSubdomains(const FaceInfo * face_info = nullptr) const;

  const bool _force_boundary_execution;

  std::unordered_set<BoundaryID> _boundaries_to_force;
  std::unordered_set<BoundaryID> _boundaries_to_not_force;

private:
  /// Computes the Jacobian contribution for every coupled variable.
  ///
  /// @param type Either ElementElement, ElementNeighbor, NeighborElement, or NeighborNeighbor. As an
  /// example ElementNeighbor means the derivatives of the elemental residual with respect to the
  /// neighbor degrees of freedom.
  ///
  /// @param residual The already computed residual (probably done with \p computeQpResidual) that
  /// also holds derivative information for filling in the Jacobians.
  void computeJacobian(Moose::DGJacobianType type, const ADReal & residual);
};
