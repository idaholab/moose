#pragma once

#include "Kernel.h"
#include "DerivativeMaterialInterfaceTHM.h"

/**
 * Computes heat source term for 1-phase flow channel coupled with a 3D heat structure.
 */
class OneD3EqnEnergyHeatFluxFromHeatStructure3D : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneD3EqnEnergyHeatFluxFromHeatStructure3D(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Creates the mapping of coupled variable index to local equation system
   * index
   */
  std::map<unsigned int, unsigned int> getVariableIndexMapping() const;

  /// User object to be queried for the average wall temperature
  const UserObject & _user_object;
  /// Heat transfer coefficient
  const MaterialProperty<Real> & _Hw;
  /// Fluid temperature
  const MaterialProperty<Real> & _T;
  /// Temperature derivatives
  const MaterialProperty<Real> & _dT_drhoA;
  const MaterialProperty<Real> & _dT_drhouA;
  const MaterialProperty<Real> & _dT_drhoEA;
  /// Coupled heated perimeter variable
  const VariableValue & _P_hf;

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rhou*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rhoE*A coupled variable index
  const unsigned int _rhoEA_jvar;
  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;

public:
  static InputParameters validParams();
};
