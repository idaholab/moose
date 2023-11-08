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

class ElementalLinearFVKernel : public LinearFVKernel, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();
  ElementalLinearFVKernel(const InputParameters & params);

  /// Compute this object's contribution to the residual
  virtual void addMatrixContribution() override;

  /// Compute this object's contribution to the diagonal Jacobian entries
  virtual void addRightHandSideContribution() override;

  /// Set current element
  void setCurrentElemInfo(const ElemInfo * elem_info) { _current_elem_info = elem_info; }

  virtual Real computeMatrixContribution() = 0;

  virtual Real computeRightHandSideContribution() = 0;

protected:
  const ElemInfo * _current_elem_info;
  MooseVariableFV<Real> & _var;
};
