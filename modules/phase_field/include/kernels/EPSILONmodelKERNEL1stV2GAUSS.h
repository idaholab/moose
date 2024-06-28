
#pragma once

#include "ADKernel.h"

class EPSILONmodelKERNEL1stV2GAUSS : public ADKernel
{
public:
  static InputParameters validParams();

  EPSILONmodelKERNEL1stV2GAUSS(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _eps;

  const ADMaterialProperty<Real> & _depsdxplus;
  const ADMaterialProperty<Real> & _depsdyplus;
  const ADMaterialProperty<Real> & _depsdzplus;

  const unsigned int _op_num;
  const std::vector<const ADVariableValue *> _vals;
  const std::vector<const ADVariableGradient *> _grad_vals;
};
