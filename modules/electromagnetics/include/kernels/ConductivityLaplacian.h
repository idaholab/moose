#pragma once

#include "ADKernel.h"

class ConductivityLaplacian : public ADKernel
{
public:
  static InputParameters validParams();

  ConductivityLaplacian(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  const ADMaterialProperty<Real> & _conductivity;
};
