#ifndef ONEDENERGYWALLHEATING_H
#define ONEDENERGYWALLHEATING_H

#include "Kernel.h"


// Forward Declarations
class OneDEnergyWallHeating;

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
  //VariableValue & _rho;
  //VariableValue & _u_vel;
  //VariableValue & _pressure;
  VariableValue & _temperature;

  // For Jacobian terms
  unsigned _rho_var_number; 

  // Parameters
  Real _Hw; // convective heat transfer coefficient, W/m^2-K
  Real _aw; // heat transfer area density, m^2 / m^3
  Real _Tw; // Wall temperature, K
};
 
#endif
