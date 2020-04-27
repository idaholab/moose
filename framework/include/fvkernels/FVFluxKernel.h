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
  /// This codifies a set of available ways to interpolate with elem+right
  /// solution information to calculate values (e.g. solution, material
  /// properties, etc.) at the face (centroid).  These methods are used in the
  /// class's interpolate functions.  Some interpolation methods are only meant
  /// to be used with advective terms (e.g. upwind), others are more
  /// generically applicable.
  enum class InterpMethod
  {
    /// (elem+right)/2
    Average,
    /// weighted
    Upwind,
  };

  static InputParameters validParams();
  FVFluxKernel(const InputParameters & params);

  /// Usually you should not override these functions - they have some super
  /// tricky stuff in them that you don't want to mess up!
  // @{
  virtual void computeResidual(const FaceInfo & fi);
  virtual void computeJacobian(const FaceInfo & fi);
  /// @}

protected:
  /// Provides interpolation of face values for non-advection-specific purposes
  /// (although it can/will still be used by advective kernels sometimes).  The
  /// interpolated value is stored in result.  This should be called when a
  /// face value needs to be computed using elem and right information (e.g. a
  /// material property, solution value, etc.).  elem and right represent the
  /// property/value to compute the face value for.
  template <typename T>
  void interpolate(InterpMethod m, T & result, const T & elem, const T & right)
  {
    switch (m)
    {
      case InterpMethod::Average:
        result = (elem + right) * 0.5;
        break;
      default:
        mooseError("unsupported interpolation method for FVFluxKernel::interpolate");
    }
  }

  /// Provides interpolation of face values for advective flux kernels.  This
  /// should be called by advective kernels when a u_face value is needed from
  /// u_elem and u_right.  The interpolated value is stored in result.  elem
  /// and right represent the property/value being advected in the elem and
  /// right elements respectively.  advector represents the vector quantity at
  /// the face that is doing the advecting (e.g. the flow velocity at the
  /// face); this value often will have been computed using a call to the
  /// non-advective interpolate function.
  template <typename T>
  void interpolate(
      InterpMethod m, T & result, const T & elem, const T & right, ADRealVectorValue advector)
  {
    switch (m)
    {
      case InterpMethod::Average:
        result = (elem + right) * 0.5;
        break;
      case InterpMethod::Upwind:
        if (advector * _normal > 0)
          result = elem;
        else
          result = right;
        break;
      default:
        mooseError("unsupported interpolation method for FVFluxKernel::interpolate");
    }
  }

  /// This is the primary function that must be implemented for flux kernel
  /// terms.  Material properties will be initialized on the face - using any
  /// reconstructed fv variable gradients if any.  Values for the solution are
  /// provided for both the elem and right side of the face.
  virtual ADReal computeQpResidual() = 0;

  /// Calculates and returns "_grad_u dot _normal" on the face to be used for
  /// diffusive terms.  If using any cross-diffusion corrections, etc. all
  /// those calculations will be handled for appropriately by this function.
  virtual ADReal gradUDotNormal();

  MooseVariableFV<Real> & _var;

  const unsigned int _qp = 0;

  /// The elem solution value of the kernel's _var for the current face.
  const ADVariableValue & _u_elem;
  /// The right solution value of the kernel's _var for the current face.
  const ADVariableValue & _u_right;
  /// The elem solution gradient of the kernel's _var for the current face.
  /// This is zero unless higher order reconstruction is used.
  const ADVariableGradient & _grad_u_elem;
  /// The right solution gradient of the kernel's _var for the current face.
  /// This is zero unless higher order reconstruction is used.
  const ADVariableGradient & _grad_u_right;

  /// This is the outward unit normal vector for the face the kernel is currently
  /// operating on.  By convention, this is set to be pointing outward from the
  /// face's elem element and residual calculations should keep this in mind.
  ADRealVectorValue _normal;

  /// This is holds meta-data for geometric information relevant to the current
  /// face including elem+right cell centroids, cell volumes, face area, etc.
  const FaceInfo * _face_info = nullptr;
};
