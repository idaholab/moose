#pragma once

#include "CoupledTimeDerivative.h"

class CoupledCoeffTimeDerivative : public CoupledTimeDerivative
{
public:
  static InputParameters validParams();

  CoupledCoeffTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _coeff;
};
