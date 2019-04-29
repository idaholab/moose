#pragma once

#include "OneDNodalBC.h"

// Forward Declarations
class OneDEnergyStaticPressureBC;
class SinglePhaseFluidProperties;
class VolumeFractionMapper;

template <>
InputParameters validParams<OneDEnergyStaticPressureBC>();

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
  Real _sign;
  const VariableValue & _alpha;
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
  unsigned _beta_var_num;

  const VariableValue & _beta;

  // Required parameters
  /// the desired input static pressure
  const Real & _p_in;

  const SinglePhaseFluidProperties & _fp;
  const VolumeFractionMapper * _vfm;
};
