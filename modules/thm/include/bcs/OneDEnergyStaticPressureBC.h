#pragma once

#include "OneDNodalBC.h"

class SinglePhaseFluidProperties;

/**
 *
 */
class OneDEnergyStaticPressureBC : public OneDNodalBC
{
public:
  OneDEnergyStaticPressureBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  bool _reversible;
  const VariableValue & _area;
  const VariableValue & _rho;
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _vel;
  const VariableValue & _vel_old;
  const VariableValue & _v_old;
  const VariableValue & _e_old;

  // Variable numbers (for Jacobians)
  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;

  // Required parameters
  /// the desired input static pressure
  const Real & _p_in;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
