#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDEnergyDensityVelocityBC;
class SinglePhaseFluidProperties;
class VolumeFractionMapper;

template <>
InputParameters validParams<OneDEnergyDensityVelocityBC>();

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

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const VariableValue & _area;
  const VariableValue & _rhoEA;

  unsigned int _beta_var_num;

  const SinglePhaseFluidProperties & _fp;
};
