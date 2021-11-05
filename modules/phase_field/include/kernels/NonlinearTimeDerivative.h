#pragma once

#include "ADTimeDerivative.h"

class NonlinearTimeDerivative : public ADTimeDerivative
{
public:
  static InputParameters validParams();
  NonlinearTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;
  const std::string & _coefficient_name;
  const ADMaterialProperty<Real> & _coefficient;
};