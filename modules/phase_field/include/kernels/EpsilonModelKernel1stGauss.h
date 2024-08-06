#pragma once

#include "ADKernel.h"

class EpsilonModelKernel1stGauss : public ADKernel
{
public:
  static InputParameters validParams();

  EpsilonModelKernel1stGauss(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  // Material property L.
  const ADMaterialProperty<Real> & _L_AD;

  // Material property epsilon.
  const ADMaterialProperty<Real> & _eps;

  // Positive derivatives of epsilon.
  const ADMaterialProperty<RealGradient> & _deps_plus;

  // Number of coupled variables.
  const unsigned int _op_num;

  // Values of the coupled variables.
  const std::vector<const ADVariableValue *> _vals;

  // Gradients of the coupled variables.
  const std::vector<const ADVariableGradient *> _grad_vals;
};
