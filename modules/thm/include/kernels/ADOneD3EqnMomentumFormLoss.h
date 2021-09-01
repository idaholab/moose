#pragma once

#include "ADKernel.h"

class Function;

/**
 * Computes the force per unit length due to form loss, provided a form
 * loss coefficient per unit length function
 *
 * See RELAP-7 Theory Manual, pg. 72, Equation (239) {eq:form_loss_force_2phase}
 */
class ADOneD3EqnMomentumFormLoss : public ADKernel
{
public:
  ADOneD3EqnMomentumFormLoss(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// area
  const ADVariableValue & _A;
  /// density
  const ADMaterialProperty<Real> & _rho;
  /// velocity
  const ADMaterialProperty<Real> & _vel;
  /// form loss coefficient per unit length function
  const ADMaterialProperty<Real> & _K_prime;

public:
  static InputParameters validParams();
};
