#pragma once

#include "ADKernel.h"

/**
 * Computes heat source term for 1-phase flow channel coupled with a 3D heat structure.
 */
class ADOneD3EqnEnergyHeatFluxFromHeatStructure3D : public ADKernel
{
public:
  ADOneD3EqnEnergyHeatFluxFromHeatStructure3D(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /**
   * Creates the mapping of coupled variable index to local equation system
   * index
   */
  std::map<unsigned int, unsigned int> getVariableIndexMapping() const;

  /// User object to be queried for the average wall temperature
  const UserObject & _user_object;
  /// Heat transfer coefficient
  const ADMaterialProperty<Real> & _Hw;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;
  /// Coupled heated perimeter variable
  const ADVariableValue & _P_hf;

public:
  static InputParameters validParams();
};
