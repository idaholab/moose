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
class FVArrayFluxBC : public FVFluxBCBase, public MooseVariableInterface<RealEigenVector>
{
public:
  FVArrayFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void computeResidual(const FaceInfo & fi) override;
  virtual void computeJacobian(const FaceInfo & fi) override;

protected:
  virtual ADRealEigenVector computeQpResidual() = 0;

  MooseVariableFV<RealEigenVector> & _var;

  const ADArrayVariableValue & _u;
};
