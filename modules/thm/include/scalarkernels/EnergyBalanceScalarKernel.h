#pragma once

#include "JunctionScalarKernel.h"

class EnergyBalanceScalarKernel;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<EnergyBalanceScalarKernel>();

/**
 *
 */
class EnergyBalanceScalarKernel : public JunctionScalarKernel
{
public:
  EnergyBalanceScalarKernel(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  unsigned int _rhoA_var_number;
  const VariableValue & _rhoA;
  unsigned int _rhouA_var_number;
  const VariableValue & _rhouA;
  unsigned int _rhoEA_var_number;
  const VariableValue & _rhoEA;
  const VariableValue & _v;
  const VariableValue & _e;
  const VariableValue & _vel;
  const VariableValue & _pressure;
  const VariableValue & _area;

  const SinglePhaseFluidProperties & _fp;
};
