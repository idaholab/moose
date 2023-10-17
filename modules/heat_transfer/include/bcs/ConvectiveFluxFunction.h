//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class ConvectiveFluxFunction : public IntegratedBC
{
public:
  static InputParameters validParams();

  ConvectiveFluxFunction(const InputParameters & parameters);
  virtual ~ConvectiveFluxFunction() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Far-field temperature
  const Function & _T_infinity;

  /// Heat transfer coefficient
  const Function & _coefficient;

  /// Enum used to define the type of function used for the heat transfer coefficient
  enum class CoefFuncType
  {
    TIME_AND_POSITION,
    TEMPERATURE
  };

  /// Type of function used for the heat transfer coefficient
  const CoefFuncType _coef_func_type;
};
