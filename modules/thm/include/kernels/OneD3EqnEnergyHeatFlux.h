#pragma once

#include "Kernel.h"

class OneD3EqnEnergyHeatFlux;
class HeatFluxFromHeatStructure3EqnUserObject;

template <>
InputParameters validParams<OneD3EqnEnergyHeatFlux>();

class OneD3EqnEnergyHeatFlux : public Kernel
{
public:
  OneD3EqnEnergyHeatFlux(const InputParameters & parameters);

  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
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

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rhou*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rhoE*A coupled variable index
  const unsigned int _rhoEA_jvar;
  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;
};
