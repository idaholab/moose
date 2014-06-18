#ifndef ONEDENERGYWALLHEATING_H
#define ONEDENERGYWALLHEATING_H

#include "Kernel.h"


// Forward Declarations
class OneDEnergyWallHeating;
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
  VariableValue & _rhoE;
  VariableValue & _temperature;
  VariableValue & _heat_transfer_coefficient;
  VariableValue & _Tw;

  // If area is coupled, we assume we're using the variable-area
  // equations.  When using the variable-area equations, there is an
  // extra (1/A) present on all the Jacobian terms.
  VariableValue & _area;

  // For Jacobian terms
  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;

  // Heat flux perimeter
  VariableValue & _Phf;

  const EquationOfState & _eos;
};

#endif //ONEDENERGYWALLHEATING_H//
