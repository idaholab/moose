#ifndef ONEDENERGYFLUX_H
#define ONEDENERGYFLUX_H

#include "Kernel.h"
#include "DerivativeMaterialInterfaceRelap.h"

class OneDEnergyFlux;

template <>
InputParameters validParams<OneDEnergyFlux>();

/**
 * Energy flux
 */
class OneDEnergyFlux : public DerivativeMaterialInterfaceRelap<Kernel>
{
public:
  OneDEnergyFlux(const InputParameters & parameters);
  virtual ~OneDEnergyFlux();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _is_liquid;
  Real _sign;
  const VariableValue & _rhouA;
  const VariableValue & _area;
  const VariableValue & _vel;
  const VariableValue & _enthalpy;
  const MaterialProperty<Real> & _pressure;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;
  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;
  bool _has_beta;
  unsigned int _beta_var_number;
  const MaterialProperty<Real> * _dp_dbeta;
  const MaterialProperty<Real> * _daL_dbeta;
  const VariableValue & _alpha;
};

#endif /* ONEDENERGYFLUX_H */
