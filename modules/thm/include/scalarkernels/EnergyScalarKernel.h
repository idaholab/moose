#pragma once

#include "NodalScalarKernel.h"

class EnergyScalarKernel;

template <>
InputParameters validParams<EnergyScalarKernel>();

/**
 * description
 */
class EnergyScalarKernel : public NodalScalarKernel
{
public:
  EnergyScalarKernel(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  unsigned int _rhoA_var_number;
  const VariableValue & _rhoA;
  unsigned int _rhouA_var_number;
  const VariableValue & _rhouA;
  unsigned int _rhoEA_var_number;
  const VariableValue & _rhoEA;
  const VariableValue & _vel;
  const VariableValue & _area;

  std::vector<Real> _normals;
  const VariableValue & _total_mfr_in;
  const VariableValue & _total_int_energy_rate_in;
};
