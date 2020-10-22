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
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"
#include "FVUtils.h"

// Provides an interface for computing residual contributions from finite
// volume numerical fluxes computed on faces to neighboring elements.
class FVFluxBCBase : public FVBoundaryCondition,
                     public CoupleableMooseVariableDependencyIntermediateInterface,
                     public MaterialPropertyInterface
{
public:
  FVFluxBCBase(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void computeResidual(const FaceInfo & fi) = 0;
  virtual void computeJacobian(const FaceInfo & fi) = 0;

protected:
  const ADRealVectorValue & normal() const { return _normal; }

  const unsigned int _qp = 0;
  ADRealVectorValue _normal;
  const FaceInfo * _face_info = nullptr;
};
