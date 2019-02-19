#ifndef ONED3EQNENERGYHEATFLUX_H
#define ONED3EQNENERGYHEATFLUX_H

#include "Kernel.h"

class OneD3EqnEnergyHeatFlux;
class HeatFluxFromHeatStructureBaseUserObject;

template <>
InputParameters validParams<OneD3EqnEnergyHeatFlux>();

class OneD3EqnEnergyHeatFlux : public Kernel
{
public:
  OneD3EqnEnergyHeatFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * Creates the mapping of coupled variable index to local equation system
   * index
   */
  std::map<unsigned int, unsigned int> getVariableIndexMapping() const;

  /// User object that computes the heat flux
  const HeatFluxFromHeatStructureBaseUserObject & _q_uo;
  /// Heat flux perimeter
  const VariableValue & _P_hf;

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rhou*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rhoE*A coupled variable index
  const unsigned int _rhoEA_jvar;
  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;
};

#endif // ONED3EQNENERGYHEATFLUX_H
