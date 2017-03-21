/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
