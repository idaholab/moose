
#pragma once

#include "ADKernel.h"

class EPSILONmodelKERNEL1stGAUSS : public ADKernel
{
public:
  static InputParameters validParams();

  EPSILONmodelKERNEL1stGAUSS(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _eps;

  const ADMaterialProperty<Real> & _depsdx;
  const ADMaterialProperty<Real> & _depsdy;
  const ADMaterialProperty<Real> & _depsdz;

  const unsigned int _op_num;
  const std::vector<const ADVariableValue *> _vals;
  const std::vector<const ADVariableGradient *> _grad_vals;

};
