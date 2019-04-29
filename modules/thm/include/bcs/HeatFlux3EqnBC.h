#pragma once

#include "IntegratedBC.h"

class HeatFlux3EqnBC;
class HeatFluxFromHeatStructure3EqnUserObject;

template <>
InputParameters validParams<HeatFlux3EqnBC>();

class HeatFlux3EqnBC : public IntegratedBC
{
public:
  HeatFlux3EqnBC(const InputParameters & parameters);

  virtual void computeJacobian() override;
  virtual void computeJacobianBlock(unsigned jvar) override;
  virtual void computeJacobianBlock(MooseVariableFEBase & jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobianNeighbor(unsigned int jvar);

  /**
   * Creates the mapping of coupled variable index to local equation system
   * index
   */
  std::map<unsigned int, unsigned int> getVariableIndexMapping() const;

  /// shape function values (in QPs)
  const VariablePhiValue & _phi_neighbor;

  /// User object that computes the heat flux
  const HeatFluxFromHeatStructure3EqnUserObject & _q_uo;
  /// Perimeter of a single unit of heat structure
  const Real _P_hs_unit;
  /// Number of units of heat structure
  const unsigned int _n_unit;
  /// Is the heat structure coordinate system cylindrical?
  const bool _hs_coord_system_is_cylindrical;
  /// Coordinate transformation
  const Real _hs_coord;
  /// Factor by which to scale term on the flow channel side for the heat structure side
  const Real _hs_scale;

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
};
