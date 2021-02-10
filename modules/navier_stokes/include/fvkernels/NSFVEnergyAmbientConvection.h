#pragma once

#include "FVElementalKernel.h"

class NSFVEnergyAmbientConvection : public FVElementalKernel
{
public:
  static InputParameters validParams();

  NSFVEnergyAmbientConvection(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// the convective heat transfer coefficient
  const ADMaterialProperty<Real> & _alpha;
  /// solid temperature
  const ADVariableValue & _temp_ambient;
};
