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

  VariableValue & _rhouA;

  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _rhoE;
  VariableValue & _u_vel;
  VariableValue & _enthalpy;

  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;

  const EquationOfState & _eos;
};


#endif /* ONEDENERGYFLUX_H */
