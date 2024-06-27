
#pragma once

#include "ADKernelGrad.h"

class ADMomentumConservativeAdvection : public ADVectorKernelGrad
{
public:
  ADMomentumConservativeAdvection(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  ADRealTensorValue precomputeQpResidual() override;

  /// This model calculates a kinematic viscosity, so rho must multiply this
  const ADMaterialProperty<Real> & _rho;
};