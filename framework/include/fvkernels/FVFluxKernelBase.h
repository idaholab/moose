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
#include "FVUtils.h"
#include "NeighborCoupleable.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborMooseVariableInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

class FaceInfo;

/// Common base class for regular (single) and array flux kernels.
class FVFluxKernelBase : public FVKernel,
                         public TwoMaterialPropertyInterface,
                         public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  static InputParameters validParams();
  FVFluxKernelBase(const InputParameters & params);

  /// Usually you should not override these functions - they have some super
  /// tricky stuff in them that you don't want to mess up!
  // @{
  virtual void computeResidual(const FaceInfo & fi) = 0;
  virtual void computeJacobian(const FaceInfo & fi) = 0;
  /// @}

protected:
  virtual MooseVariableFieldBase & fieldVar() = 0;

  const ADRealVectorValue & normal() const { return _normal; }

  const unsigned int _qp = 0;

  /// This is the outward unit normal vector for the face the kernel is currently
  /// operating on.  By convention, this is set to be pointing outward from the
  /// face's elem element and residual calculations should keep this in mind.
  ADRealVectorValue _normal;

  /// This is holds meta-data for geometric information relevant to the current
  /// face including elem+neighbor cell centroids, cell volumes, face area, etc.
  const FaceInfo * _face_info = nullptr;

  /// Kernels are called even on boundaries in case one is for a variable with
  /// a dirichlet BC - in which case we need to run the kernel with a
  /// ghost-element.  This returns true if we need to run because of dirichlet
  /// conditions - otherwise this returns false and all jacobian/residual calcs
  /// should be skipped.
  bool skipForBoundary(const FaceInfo & fi);

  const bool _force_boundary_execution;
};
