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

class FVKernel : public MooseObject,
                 public TaggingInterface,
                 public TransientInterface,
                 public BlockRestrictable
{
public:
  static InputParameters validParams();
  FVKernel(const InputParameters & params);
};

class FVFluxKernel : public FVKernel,
                     public TwoMaterialPropertyInterface,
                     public NeighborMooseVariableInterface<Real>,
                     public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();
  FVFluxKernel(const InputParameters & params);

  virtual void computeResidual(const FaceInfo & fi);

protected:
  // material properties will be initialized on the face.  Reconstructed
  // solutions will have been performed previous to this call and all coupled
  // variables and _u, _grad_u, etc. will have their reconstructed values
  // extrapolated to/at the face.  Material properties will also have been
  // computed on the face using the face-reconstructed variable values.
  //
  virtual Real computeQpResidual() = 0;

  MooseVariableFV<Real> & _var;
  THREAD_ID _tid;
  Assembly & _assembly;

  const unsigned int _qp = 0;
  const VariableValue & _u_left;
  const VariableValue & _u_right;
  const VariableGradient & _grad_u_left;
  const VariableGradient & _grad_u_right;
  Point _normal;

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
