#ifndef ONEDENERGYFLUX_H
#define ONEDENERGYFLUX_H

#include "Kernel.h"

class OneDEnergyFlux;
class EquationOfState;

template<>
InputParameters validParams<OneDEnergyFlux>();

/**
 * Energy flux
 */
class OneDEnergyFlux : public Kernel
{
public:
  OneDEnergyFlux(const std::string & name, InputParameters parameters);
  virtual ~OneDEnergyFlux();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  bool _is_liquid;
  Real _sign;

  VariableValue & _rhouA;

  VariableValue & _area;
  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _rhoE;
  VariableValue & _u_vel;
  VariableValue & _enthalpy;
  VariableValue & _pressure;

  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;
  bool _has_alpha_A;
  unsigned int _alpha_A_liquid_var_number;
  MaterialProperty<Real> * _dp_dalphaA_liquid;

  VariableValue & _alpha;

  const EquationOfState & _eos;
};


#endif /* ONEDENERGYFLUX_H */
