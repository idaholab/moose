//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideVectorPostprocessor.h"
#include "OptimizationFunctionInnerProductHelper.h"

class SideOptimizationFunctionInnerProduct : public SideVectorPostprocessor,
                                             public OptimizationFunctionInnerProductHelper
{
public:
  static InputParameters validParams();
  SideOptimizationFunctionInnerProduct(const InputParameters & parameters);

  virtual void initialize() override final;
  virtual void execute() override final;
  virtual void threadJoin(const UserObject & uo) override final;
  virtual void finalize() override final;

protected:
  /**
   * Used to compute the inner product at a certain quadrature point.
   * Function is already considered in in this parent class.
   */
  virtual Real computeQpInnerProduct() = 0;

  /// Quadrature point values of coupled variable
  const VariableValue & _var;
  /// Vector holding inner product
  VectorPostprocessorValue & _vec;
  /// Quadrature point index
  unsigned int _qp;
};
