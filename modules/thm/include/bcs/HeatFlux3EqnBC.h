#pragma once

#include "HeatFluxBaseBC.h"

class HeatFlux3EqnBC;

template <>
InputParameters validParams<HeatFlux3EqnBC>();

class HeatFlux3EqnBC : public HeatFluxBaseBC
{
public:
  HeatFlux3EqnBC(const InputParameters & parameters);

protected:
  virtual std::vector<unsigned int> getOffDiagVariableNumbers() override;

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobianNeighbor(unsigned int jvar) override;

  /**
   * Creates the mapping of coupled variable index to local equation system
   * index
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
};
