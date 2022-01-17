#pragma once

#include "IntegratedBC.h"

class HeatTransferFromHeatStructure3D1PhaseUserObject;

/**
 * Convective boundary condition from a single-phase flow channel for a 3D heat structure.
 */
class ConvectionHeatTransfer3DBC : public IntegratedBC
{
public:
  ConvectionHeatTransfer3DBC(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual std::vector<unsigned int> getOffDiagVariableNumbers();
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;
  virtual Real computeQpOffDiagJacobianNeighbor(unsigned int jvar);

  /// User object that computes the heat transfer data (fluid temperature and heat transfer coefficient)
  const HeatTransferFromHeatStructure3D1PhaseUserObject & _ht_uo;

  /**
   * Creates the mapping of coupled variable
   * index to local equation system index
   */
  std::map<unsigned int, unsigned int> getVariableIndexMapping() const;

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rhou*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rhoE*A coupled variable index
  const unsigned int _rhoEA_jvar;
  /// Wall temperature coupled variable index
  const unsigned int _T_wall_jvar;
  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;
  /// shape function values (in QPs)
  const VariablePhiValue & _phi_neighbor;
  /// Variable numbers for the off-diagonal jacobian computation
  std::vector<unsigned int> _off_diag_var_nums;

public:
  static InputParameters validParams();
};
