#pragma once

#include "FVKernel.h"
#include "NeighborCoupleable.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborMooseVariableInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

class FaceInfo;

class FVFluxKernel : public FVKernel,
                     public TwoMaterialPropertyInterface,
                     public NeighborMooseVariableInterface<Real>,
                     public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();
  FVFluxKernel(const InputParameters & params);

  virtual void computeResidual(const FaceInfo & fi);
  virtual void computeJacobian(const FaceInfo & fi);

protected:
  // material properties will be initialized on the face.  Reconstructed
  // solutions will have been performed previous to this call and all coupled
  // variables and _u, _grad_u, etc. will have their reconstructed values
  // extrapolated to/at the face.  Material properties will also have been
  // computed on the face using the face-reconstructed variable values.
  virtual ADReal computeQpResidual() = 0;

  /// Calculates and returns "_grad_u dot _normal" used for diffusive terms.
  /// If using any cross-diffusion corrections, etc. all those calculations
  /// will be handled for appropriately by this function.
  virtual ADReal gradUDotNormal();

  MooseVariableFV<Real> & _var;

  const unsigned int _qp = 0;
  const ADVariableValue & _u_left;
  const ADVariableValue & _u_right;
  const ADVariableGradient & _grad_u_left;
  const ADVariableGradient & _grad_u_right;
  ADRealVectorValue _normal;
  const FaceInfo * _face_info = nullptr;
};
