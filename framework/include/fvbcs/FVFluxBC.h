//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVBoundaryCondition.h"

// this intermediate class exists so we can call computeResidual/Jacobian on
// flux bcs without knowing their AD compute_stage template type
// parameter.
class FVFluxBCBase : public FVBoundaryCondition
{
public:
  static InputParameters validParams();
  FVFluxBCBase(const InputParameters & params);

  virtual void computeResidual(const FaceInfo & fi) = 0;
  virtual void computeJacobian(const FaceInfo & fi) = 0;
};

#define usingFVFluxBCMembers                                                                   \
  using FVFluxBC<compute_stage>::_qp;                                                          \
  using FVFluxBC<compute_stage>::_u;                                                           \
  using FVFluxBC<compute_stage>::_assembly;                                                    \
  using FVFluxBC<compute_stage>::_tid;                                                         \
  using FVFluxBC<compute_stage>::_var;                                                         \
  using FVFluxBC<compute_stage>::_normal;                                                      \
  using FVFluxBC<compute_stage>::_face_info

/**
 * Base class for Finite Volume Flux BCs
 */
template <ComputeStage compute_stage>
class FVFluxBC : public FVFluxBCBase
{
public:
  FVFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void computeResidual(const FaceInfo & fi) override;
  virtual void computeJacobian(const FaceInfo & fi) override;

protected:
  virtual ADReal computeQpResidual() = 0;

  const unsigned int _qp = 0;
  const ADVariableValue & _u;
  // TODO: gradients
  ADRealVectorValue _normal;
  const FaceInfo * _face_info = nullptr;
};
