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

class LinearFVElementalKernel : public LinearFVKernel
{
public:
  static InputParameters validParams();
  LinearFVElementalKernel(const InputParameters & params);

  /// Compute this object's contribution to the system matrix
  virtual void addMatrixContribution() override;

  /// Compute this object's contribution to the right hand side
  virtual void addRightHandSideContribution() override;

  /// Set current element
  void setCurrentElemInfo(const ElemInfo * elem_info) { _current_elem_info = elem_info; }

  /// Computes the system matrix contribution for the current element
  virtual Real computeMatrixContribution() = 0;

  /// Computes the right hand side contribution for the current element
  virtual Real computeRightHandSideContribution() = 0;

protected:
  /// Pointer to the current element info
  const ElemInfo * _current_elem_info;
};
