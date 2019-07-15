#pragma once

#include "OneDHeatFluxBase.h"

class OneD3EqnEnergyHeatFlux;

template <>
InputParameters validParams<OneD3EqnEnergyHeatFlux>();

class OneD3EqnEnergyHeatFlux : public OneDHeatFluxBase
{
public:
  OneD3EqnEnergyHeatFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
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
  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;
};
