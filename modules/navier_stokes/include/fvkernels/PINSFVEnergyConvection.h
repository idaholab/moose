#pragma once

#include "FVElementalKernel.h"

class INSFVEnergyVariable;

class PINSFVEnergyConvection : public FVElementalKernel
{
public:
  static InputParameters validParams();

  PINSFVEnergyConvection(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// porosity of the local medium
  const VariableValue & _eps;
  /// the convective heat transfer coefficient
  const ADMaterialProperty<Real> & _h_solid_fluid;
  /// fluid temperature
  const ADVariableValue & _temp_fluid;
  /// solid temperature
  const ADVariableValue & _temp_solid;
  /// whether this kernel is being used for a solid or a fluid temperature
  const bool _is_solid;
};
