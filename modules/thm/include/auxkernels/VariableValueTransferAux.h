#pragma once

#include "NearestNodeValueAux.h"

class VariableValueTransferAux;
class PenetrationLocator;
class NearestNodeLocator;

template <>
InputParameters validParams<VariableValueTransferAux>();

/**
 * Transfer variable values from a surface of a 2D mesh onto 1D mesh
 */
class VariableValueTransferAux : public AuxKernel
{
public:
  VariableValueTransferAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  PenetrationLocator & _penetration_locator;
  NearestNodeLocator & _nearest_node;

  const NumericVector<Number> *& _serialized_solution;

  unsigned int _paired_variable;
};
