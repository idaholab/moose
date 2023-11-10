//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVKernel.h"
#include "ElemInfo.h"
#include "MooseVariableFV.h"
#include "MooseVariableInterface.h"

class FluxLinearFVKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();
  FluxLinearFVKernel(const InputParameters & params);

  /// Compute this object's contribution to the residual
  virtual void addMatrixContribution() override;

  /// Compute this object's contribution to the diagonal Jacobian entries
  virtual void addRightHandSideContribution() override;

  /// Set current element
  void setCurrentFaceInfo(const FaceInfo * face_info) { _current_face_info = face_info; }

  virtual Real computeElemMatrixContribution() = 0;

  virtual Real computeNeighborMatrixContribution() = 0;

  virtual Real computeElemRightHandSideContribution() = 0;

  virtual Real computeNeighborRightHandSideContribution() = 0;

protected:
  const FaceInfo * _current_face_info;
};
