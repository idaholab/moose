#pragma once

#include "FVElementalKernel.h"

class INSFVEnergyVariable;

class NSFVEnergyConvection : public FVElementalKernel
{
public:
  static InputParameters validParams();

  NSFVEnergyConvection(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// the convective heat transfer coefficient
  const ADMaterialProperty<Real> & _h_solid_fluid;
  /// fluid temperature
  const ADVariableValue & _temp_fluid;
  /// solid temperature
  const ADVariableValue & _temp_solid;
};
