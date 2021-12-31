//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

class DiffusionVariableIntegral : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  DiffusionVariableIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
  ///@{ Derivative of materwith with respect to parameter being optimized
  const MaterialProperty<Real> & _dMdP;
  ///@}
  /// Holds variable1 gradient at the current quadrature points
  const VariableGradient & _grad_u;
  /// Holds variable2 gradient at the current quadrature points
  const VariableGradient & _grad_v;
};
