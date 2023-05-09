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
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "MathFVUtils.h"
#include "FVFaceResidualObject.h"

/**
 * Provides an interface for computing residual contributions from finite
 * volume numerical fluxes computed on faces to neighboring elements.
 */
class FVFluxBC : public FVBoundaryCondition,
                 public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                 public TwoMaterialPropertyInterface,
                 public FVFaceResidualObject
{
public:
  FVFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  void computeResidual(const FaceInfo & fi) override;
  void computeJacobian(const FaceInfo & fi) override;
  void computeResidualAndJacobian(const FaceInfo & fi) override;

protected:
  virtual ADReal computeQpResidual() = 0;

  const ADRealVectorValue & normal() const { return _normal; }

  const unsigned int _qp = 0;
  const ADVariableValue & _u;
  const ADVariableValue & _u_neighbor;
  // TODO: add gradients once we have reconstruction.
  ADRealVectorValue _normal;

  /**
   * @return the value of u at the cell centroid on the subdomain on which u is defined. E.g. u is
   * defined on either the FaceInfo elem or FacInfo neighbor subdomain
   */
  const ADReal & uOnUSub() const;

  /**
   * @return the value of u at the ghost cell centroid
   */
  const ADReal & uOnGhost() const;

  /**
   * @return an element argument corresponding to the face info elem
   */
  Moose::ElemArg elemArg(bool correct_skewness = false) const;

  /**
   * @return an element argument corresponding to the face info neighbor
   */
  Moose::ElemArg neighborArg(bool correct_skewness = false) const;

  /// The variable face type
  FaceInfo::VarFaceNeighbors _face_type;
};
