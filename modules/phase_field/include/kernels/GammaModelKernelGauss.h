#pragma once

#include "ADKernel.h"

class GammaModelKernelGauss : public ADKernel
{
public:
  static InputParameters validParams();

  GammaModelKernelGauss(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  // Material property L.
  const ADMaterialProperty<Real> & _L_AD;

  // Material property m.
  const ADMaterialProperty<Real> & _m;

  // Positive derivatives of gamma.
  const ADMaterialProperty<RealGradient> & _dgamma_plus;

  // Number of coupled order parameters.
  const unsigned int _op_num;

  // Values of the coupled variables.
  const std::vector<const ADVariableValue *> _vals;
};
