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

/**
 * Base class for Finite Volume Flux BCs
 */
class FVFluxBC : public FVBoundaryCondition
{
public:
  FVFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void computeResidual(const FaceInfo & fi);
  virtual void computeJacobian(const FaceInfo & fi);

protected:
  virtual ADReal computeQpResidual() = 0;

  const unsigned int _qp = 0;
  const ADVariableValue & _u;
  // TODO: gradients
  ADRealVectorValue _normal;
  const FaceInfo * _face_info = nullptr;
};
