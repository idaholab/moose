//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSENTROPYERROR_H
#define NSENTROPYERROR_H

#include "ElementIntegralPostprocessor.h"

// Forward Declarations
class NSEntropyError;
class IdealGasFluidProperties;

template <>
InputParameters validParams<NSEntropyError>();

class NSEntropyError : public ElementIntegralPostprocessor
{
public:
  NSEntropyError(const InputParameters & parameters);
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

  Real _rho_infty;
  Real _p_infty;

  const VariableValue & _rho;
  const VariableValue & _pressure;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};

#endif
