#pragma once

#include "OneDIntegratedBC.h"
#include "FlowModel.h"

class VolumeJunctionOldBC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<VolumeJunctionOldBC>();

class VolumeJunctionOldBC : public OneDIntegratedBC
{
public:
  VolumeJunctionOldBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  /// which equation (mass/momentum/energy) this BC is acting on
  FlowModel::EEquationType _eqn_type;
  // Coupled variables
  const VariableValue & _area;
  unsigned int _rhoA_var_number;
  const VariableValue & _rhoA;
  const VariableValue & _rho;
  unsigned int _rhouA_var_number;
  const VariableValue & _rhoEA;
  const VariableValue & _vel;

  // Coupled VolumeJunctionOld variables
  unsigned int _rho_junction_var_number;
  const VariableValue & _rho_junction;
  unsigned int _rhoe_junction_var_number;
  const VariableValue & _rhoe_junction;
  unsigned int _vel_junction_var_number;
  const VariableValue & _vel_junction;
  const VariableValue & _pressure_junction;
  const VariableValue & _total_mfr_in;

  const Real & _k_coeff;
  const Real & _kr_coeff;
  const Real & _ref_area;
  Real _deltaH;

  /// Gravitational acceleration magnitude
  const Real & _gravity_magnitude;

  const SinglePhaseFluidProperties & _fp;
};
