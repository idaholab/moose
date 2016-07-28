#ifndef ONEDMOMENTUMFRICTION_H
#define ONEDMOMENTUMFRICTION_H

#include "Kernel.h"

// Forward Declarations
class OneDMomentumFriction;

template<>
InputParameters validParams<OneDMomentumFriction>();

/**
 * Contribution due to wall friction
 */
class OneDMomentumFriction : public Kernel
{
public:
  OneDMomentumFriction(const InputParameters & parameters);
  virtual ~OneDMomentumFriction();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _area;
  const VariableValue & _u_vel;
  const VariableValue & _rhoA;
  const MaterialProperty<Real> & _Cw;
  const MaterialProperty<Real> * const _dCw_dbeta;
  const MaterialProperty<Real> & _dCw_drhoA;
  const MaterialProperty<Real> & _dCw_drhouA;
  const MaterialProperty<Real> & _dCw_drhoEA;

  unsigned int _beta_var_number;
  unsigned int _rhoA_var_number;
  unsigned int _rhouA_var_number;
  unsigned int _rhoEA_var_number;
};

#endif
