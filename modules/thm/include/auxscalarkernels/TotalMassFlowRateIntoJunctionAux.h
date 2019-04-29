#pragma once

#include "AuxNodalScalarKernel.h"

class TotalMassFlowRateIntoJunctionAux;

template <>
InputParameters validParams<TotalMassFlowRateIntoJunctionAux>();

/**
 * Computes total mass flow rate going into the junction
 */
class TotalMassFlowRateIntoJunctionAux : public AuxNodalScalarKernel
{
public:
  TotalMassFlowRateIntoJunctionAux(const InputParameters & parameters);

  virtual Real computeValue();

protected:
  const VariableValue & _rhouA;
  /// Component of outward normals along 1-D direction at nodes
  const std::vector<Real> & _normals;
};
