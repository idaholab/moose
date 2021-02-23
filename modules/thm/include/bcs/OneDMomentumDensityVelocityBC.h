#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"
#include "Function.h"

class SinglePhaseFluidProperties;

/**
 *
 */
class OneDMomentumDensityVelocityBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDMomentumDensityVelocityBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real & _rho;
  const Real & _vel;

  const VariableValue & _area;
  const VariableValue & _rhoEA;

  unsigned int _rhoEA_var_num;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
