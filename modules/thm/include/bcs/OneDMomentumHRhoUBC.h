#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDMomentumHRhoUBC;

template <>
InputParameters validParams<OneDMomentumHRhoUBC>();

/**
 *
 */
class OneDMomentumHRhoUBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMomentumHRhoUBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _is_liquid;
  Real _sign;
  const Real & _rhou;
  const VariableValue & _alpha;
  const VariableValue & _area;
  const VariableValue & _arhoA;
  const VariableValue & _arhoEA;
  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;
  const MaterialProperty<Real> * _dp_dbeta;

  unsigned int _arhoA_var_num;
  unsigned int _arhoEA_var_num;
  unsigned int _beta_var_num;
};
