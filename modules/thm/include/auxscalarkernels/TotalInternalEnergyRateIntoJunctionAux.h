#pragma once

#include "AuxNodalScalarKernel.h"

class TotalInternalEnergyRateIntoJunctionAux;

template <>
InputParameters validParams<TotalInternalEnergyRateIntoJunctionAux>();

/**
 * Computes total internal energy rate in the junction (only 1D version)
 */
class TotalInternalEnergyRateIntoJunctionAux : public AuxNodalScalarKernel
{
public:
  TotalInternalEnergyRateIntoJunctionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;
  /// Component of outward normals along 1-D direction at nodes
  const std::vector<Real> & _normals;
};
