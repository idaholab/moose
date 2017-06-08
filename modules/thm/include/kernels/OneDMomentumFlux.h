#ifndef ONEDMOMENTUMFLUX_H
#define ONEDMOMENTUMFLUX_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceRelap.h"

class OneDMomentumFlux;

template <>
InputParameters validParams<OneDMomentumFlux>();

/**
 * Momentum flux
 */
class OneDMomentumFlux : public DerivativeMaterialInterfaceRelap<Kernel>
{
public:
  OneDMomentumFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const VariableValue & _vel;
  const MaterialProperty<Real> & _pressure;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;
  const VariableValue & _area;

  unsigned int _rhoA_var_number;
  unsigned int _rhoEA_var_number;

  const VariableValue & _beta;
  unsigned int _beta_var_number;
  const MaterialProperty<Real> * _dp_dbeta;
};

#endif /* ONEDMOMENTUMFLUX_H */
