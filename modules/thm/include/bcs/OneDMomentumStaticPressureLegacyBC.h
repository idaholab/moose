#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

// Forward Declarations
class OneDMomentumStaticPressureLegacyBC;

template <>
InputParameters validParams<OneDMomentumStaticPressureLegacyBC>();

/**
 *
 */
class OneDMomentumStaticPressureLegacyBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMomentumStaticPressureLegacyBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _reversible;
  unsigned int _rhoA_var_number;
  unsigned int _rhoEA_var_number;
  unsigned int _beta_var_num;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  // Coupled variables
  const VariableValue & _area;
  const VariableValue & _rhoA;
  const VariableValue & _vel_old;
  // Required parameters
  const Real & _p_in;
};
