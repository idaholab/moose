#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

// Forward Declarations
class OneDEnergyWallHeating;

template <>
InputParameters validParams<OneDEnergyWallHeating>();

// The spatial part of the 1D energy conservation for Navier-Stokes flow
class OneDEnergyWallHeating : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneDEnergyWallHeating(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _temperature;
  const MaterialProperty<Real> & _dT_drhoA;
  const MaterialProperty<Real> & _dT_drhouA;
  const MaterialProperty<Real> & _dT_drhoEA;
  const MaterialProperty<Real> & _Hw;

  const VariableValue & _T_wall;
  const VariableValue & _P_hf;

  // For Jacobian terms
  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;
};
