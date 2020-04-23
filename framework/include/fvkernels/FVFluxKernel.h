#pragma once

#include "FVKernel.h"
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

protected:
  /// This is the primary function that must be implemented for flux kernel
  /// terms.  Material properties will be initialized on the face - using any
  /// reconstructed fv variable gradients if any.  Values for the solution are
  /// provided for both the left and right side of the face.
  virtual ADReal computeQpResidual() = 0;

  /// Calculates and returns "_grad_u dot _normal" on the face to be used for
  /// diffusive terms.  If using any cross-diffusion corrections, etc. all
  /// those calculations will be handled for appropriately by this function.
  virtual ADReal gradUDotNormal();

  MooseVariableFV<Real> & _var;

  const unsigned int _qp = 0;

  /// The left solution value of the kernel's _var for the current face.
  const ADVariableValue & _u_left;
  /// The right solution value of the kernel's _var for the current face.
  const ADVariableValue & _u_right;
  /// The left solution gradient of the kernel's _var for the current face.
  /// This is zero unless higher order reconstruction is used.
  const ADVariableGradient & _grad_u_left;
  /// The right solution gradient of the kernel's _var for the current face.
  /// This is zero unless higher order reconstruction is used.
  const ADVariableGradient & _grad_u_right;

  /// This is the outward unit normal vector for the face the kernel is currently
  /// operating on.  By convention, this is set to be pointing outward from the
  /// face's left element and residual calculations should keep this in mind.
  ADRealVectorValue _normal;

  /// This is holds meta-data for geometric information relevant to the current
  /// face including left+right cell centroids, cell volumes, face area, etc.
  const FaceInfo * _face_info = nullptr;
};
