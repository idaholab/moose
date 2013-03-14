#ifndef ONEDENERGYWALLHEATING_H
#define ONEDENERGYWALLHEATING_H

#include "Kernel.h"

// Forward Declarations
class OneDEnergyWallHeating;
//class Function;
class EquationOfState;

template<>
InputParameters validParams<OneDEnergyWallHeating>();

// The spatial part of the 1D energy conservation for Navier-Stokes flow
class OneDEnergyWallHeating : public Kernel
{
public:

  OneDEnergyWallHeating(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variables
  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _temperature;
  VariableValue & _HTC_aux; // convective heat transfer coefficient, W/m^2-K 

  // For Jacobian terms
  unsigned _rho_var_number; 
  unsigned _rhou_var_number; 

  // Parameters
  /// heat transfer area density, m^2 / m^3
  const Real & _aw;
  /// Wall temperature, K
  Real _Tw;

  const EquationOfState & _eos;
};
 
#endif
