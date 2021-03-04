#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

class SinglePhaseFluidProperties;

/**
 *
 */
class OneDEnergyDensityVelocityBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDEnergyDensityVelocityBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real & _rho;
  const Real & _vel;

  const VariableValue & _area;
  const VariableValue & _rhoEA;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
