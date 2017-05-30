#ifndef ONEDMASSFLUX_H
#define ONEDMASSFLUX_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceRelap.h"

class OneDMassFlux;

template <>
InputParameters validParams<OneDMassFlux>();

/**
 * Mass flux
 */
class OneDMassFlux : public DerivativeMaterialInterfaceRelap<Kernel>
{
public:
  OneDMassFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _A;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> * const _drho_dbeta;
  const MaterialProperty<Real> & _drho_darhoA;

  const MaterialProperty<Real> & _vel;
  const MaterialProperty<Real> & _dvel_darhoA;
  const MaterialProperty<Real> & _dvel_darhouA;

  const unsigned int _beta_var_number;
  const unsigned int _arhouA_var_number;
};

#endif /* ONEDMASSFLUX_H */
