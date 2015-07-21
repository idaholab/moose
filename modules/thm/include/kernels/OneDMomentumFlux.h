#ifndef ONEDMOMENTUMFLUX_H
#define ONEDMOMENTUMFLUX_H

#include "Kernel.h"

class OneDMomentumFlux;

template<>
InputParameters validParams<OneDMomentumFlux>();

/**
 * Momentum flux
 */
class OneDMomentumFlux : public Kernel
{
public:
  OneDMomentumFlux(const InputParameters & parameters);
  virtual ~OneDMomentumFlux();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _is_liquid;
  Real _sign;
  VariableValue & _alpha;
  VariableValue & _u_vel;
  const MaterialProperty<Real> & _pressure;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;
  VariableValue & _area;

  unsigned int _rhoA_var_number;
  unsigned int _rhoEA_var_number;

  bool _has_alpha_A;
  unsigned int _alpha_A_liquid_var_number;
  const MaterialProperty<Real> * _dp_daAL;
};

#endif /* ONEDMOMENTUMFLUX_H */
