#pragma once

#include "OneDIntegratedBC.h"
#include "DerivativeMaterialInterfaceTHM.h"

class OneDEnergyMassFlowRateTemperatureBC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<OneDEnergyMassFlowRateTemperatureBC>();

/**
 * Mass flow rate (m_dot) and temperature (T) BC
 */
class OneDEnergyMassFlowRateTemperatureBC : public DerivativeMaterialInterfaceTHM<OneDIntegratedBC>
{
public:
  OneDEnergyMassFlowRateTemperatureBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real & _m_dot;
  const Real & _T;

  const MaterialProperty<Real> & _alpha;
  const MaterialProperty<Real> * const _dalpha_dbeta;

  const VariableValue & _area;
  const MaterialProperty<Real> & _p;
  const MaterialProperty<Real> & _dp_darhoA;
  const MaterialProperty<Real> & _dp_darhouA;
  const MaterialProperty<Real> & _dp_darhoEA;
  const MaterialProperty<Real> * _dp_dbeta;

  unsigned int _arhoA_var_num;
  unsigned int _arhouA_var_num;
  unsigned int _beta_var_num;

  const SinglePhaseFluidProperties & _fp;
};
