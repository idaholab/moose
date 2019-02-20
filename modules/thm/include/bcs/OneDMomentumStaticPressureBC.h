#ifndef ONEDMOMENTUMSTATICPRESSUREBC_H
#define ONEDMOMENTUMSTATICPRESSUREBC_H

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

// Forward Declarations
class OneDMomentumStaticPressureBC;

template <>
InputParameters validParams<OneDMomentumStaticPressureBC>();

/**
 *
 */
class OneDMomentumStaticPressureBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMomentumStaticPressureBC(const InputParameters & parameters);

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

#endif // ONEDMOMENTUMSTATICPRESSUREBC_H
