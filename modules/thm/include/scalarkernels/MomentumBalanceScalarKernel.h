#pragma once

#include "JunctionScalarKernel.h"

class MomentumBalanceScalarKernel;

template <>
InputParameters validParams<MomentumBalanceScalarKernel>();

/**
 *
 */
class MomentumBalanceScalarKernel : public JunctionScalarKernel
{
public:
  MomentumBalanceScalarKernel(const InputParameters & parameters);

  virtual void computeResidual();
  virtual void computeJacobian();

protected:
  unsigned int _rhoA_var_number;
  unsigned int _rhouA_var_number;
  unsigned int _rhoEA_var_number;
  const VariableValue & _vel;

  const VariableValue & _junction_rho;
  unsigned int _junction_rho_var_number;
  const VariableValue & _total_mfr_in;

  const Real & _ref_area;
};
