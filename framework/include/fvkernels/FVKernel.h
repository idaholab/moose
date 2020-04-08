#pragma once

#include "MooseObject.h"
#include "TaggingInterface.h"
#include "TransientInterface.h"
#include "BlockRestrictable.h"
#include "NeighborCoupleable.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborMooseVariableInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

class FaceInfo;
class SubProblem;

class FVKernel : public MooseObject,
                 public TaggingInterface,
                 public TransientInterface,
                 public BlockRestrictable
{
public:
  static InputParameters validParams();
  FVKernel(const InputParameters & params);

protected:
  SubProblem & _subproblem;
};

#define usingFVFluxKernelMembers                                                                   \
  using FVFluxKernel<compute_stage>::_qp;                                                          \
  using FVFluxKernel<compute_stage>::_u_left;                                                      \
  using FVFluxKernel<compute_stage>::_u_right;                                                     \
  using FVFluxKernel<compute_stage>::_grad_u_left;                                                 \
  using FVFluxKernel<compute_stage>::_grad_u_right;                                                \
  using FVFluxKernel<compute_stage>::_assembly;                                                    \
  using FVFluxKernel<compute_stage>::_tid;                                                         \
  using FVFluxKernel<compute_stage>::_var;                                                         \
  using FVFluxKernel<compute_stage>::_normal;                                                      \
  using FVFluxKernel<compute_stage>::_face_info

// this intermediate class exists so we can call computeResidual/Jacobian on
// flux kernels without knowing their AD compute_stage template type
// parameter.
class FVFluxKernelBase : public FVKernel,
                         public TwoMaterialPropertyInterface,
                         public NeighborMooseVariableInterface<Real>,
                         public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();
  FVFluxKernelBase(const InputParameters & params);

  virtual void computeResidual(const FaceInfo & fi) = 0;
  virtual void computeJacobian(const FaceInfo & fi) = 0;
};

template <ComputeStage compute_stage>
class FVFluxKernel : public FVFluxKernelBase
{
public:
  static InputParameters validParams();
  FVFluxKernel(const InputParameters & params);

  virtual void computeResidual(const FaceInfo & fi) override;
  virtual void computeJacobian(const FaceInfo & fi) override;

protected:
  // material properties will be initialized on the face.  Reconstructed
  // solutions will have been performed previous to this call and all coupled
  // variables and _u, _grad_u, etc. will have their reconstructed values
  // extrapolated to/at the face.  Material properties will also have been
  // computed on the face using the face-reconstructed variable values.
  //
  virtual ADReal computeQpResidual() = 0;

  virtual ADReal computeQpJacobian() = 0;

  MooseVariableFV<Real> & _var;
  THREAD_ID _tid;
  Assembly & _assembly;

  const unsigned int _qp = 0;
  const ADVariableValue & _u_left;
  const ADVariableValue & _u_right;
  const ADVariableGradient & _grad_u_left;
  const ADVariableGradient & _grad_u_right;
  ADRealVectorValue _normal;
  const FaceInfo * _face_info = nullptr;

private:
  bool ownLeftElem()
  {
    // returns true if this processor owns the (left) element.
    // TODO: implement this
    return true;
  }
  bool ownRightElem()
  {
    // returns true if this processor owns the (right) neighbor.
    // TODO: implement this
    return true;
  }
};
