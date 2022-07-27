//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVariableVectorPostprocessor.h"

#include "OptimizationFunction.h"

class ElementOptimizationSourceFunctionInnerProduct : public ElementVariableVectorPostprocessor
{
public:
  static InputParameters validParams();
  ElementOptimizationSourceFunctionInnerProduct(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

protected:
  /// Quadrature point values of coupled MOOSE variables
  const VariableValue & _var;
  /// Function used for source
  const OptimizationFunction * const _function;
  /// Vector holding inner product
  VectorPostprocessorValue & _vec;
  /// The final time when we want to reverse the time index in function evaluation
  const Real & _reverse_time_end;

private:
  /// Vector holding data for each time
  std::vector<std::pair<Real, std::vector<Real>>> _time_ip;
  /// Vector for current time
  std::vector<Real> * _curr_time_ip;
};
