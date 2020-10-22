//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBCBase.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MaterialPropertyInterface.h"
#include "FVUtils.h"

// Provides an interface for computing residual contributions from finite
// volume numerical fluxes computed on faces to neighboring elements.
class FVFluxBC : public FVFluxBCBase, public MooseVariableInterface<Real>
{
public:
  FVFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void computeResidual(const FaceInfo & fi) override;
  virtual void computeJacobian(const FaceInfo & fi) override;

protected:
  virtual ADReal computeQpResidual() = 0;

  MooseVariableFV<Real> & _var;

  const ADVariableValue & _u;

private:
  /// Computes the Jacobian contribution for every coupled variable.
  ///
  /// @param type Either ElementElement, ElementNeighbor, NeighborElement, or NeighborNeighbor. As an
  /// example ElementNeighbor means the derivatives of the elemental residual with respect to the
  /// neighbor degrees of freedom
  ///
  /// @param residual The already computed residual (probably done with \p computeQpResidual) that
  /// also holds derivative information for filling in the Jacobians
  void computeJacobian(Moose::DGJacobianType type, const ADReal & residual);
};
