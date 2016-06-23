#ifndef SCALARCOUPLEDPOSTPROCESSOR_H
#define SCALARCOUPLEDPOSTPROCESSOR_H

#include "SideIntegralPostprocessor.h"

class ScalarCoupledPostprocessor;

template<>
InputParameters validParams<ScalarCoupledPostprocessor>();

class ScalarCoupledPostprocessor : public SideIntegralPostprocessor
{
public:
  ScalarCoupledPostprocessor(const InputParameters & parameters);

protected:

  Real computeQpIntegral();

  const VariableValue & _coupled_scalar;
  const VariableValue & _u;
};

#endif
