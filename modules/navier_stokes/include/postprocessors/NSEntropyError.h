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

// Forward Declarations
class IdealGasFluidProperties;

class NSEntropyError : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  NSEntropyError(const InputParameters & parameters);
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;

  Real _rho_infty;
  Real _p_infty;

  const VariableValue & _rho;
  const VariableValue & _pressure;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};
