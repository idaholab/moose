
#pragma once

#include "ADKernel.h"

class GAMMAmodelKERNELV2GAUSS : public ADKernel
{
public:
  static InputParameters validParams();

  GAMMAmodelKERNELV2GAUSS(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _L_AD;

  const ADMaterialProperty<Real> & _m;

  const ADMaterialProperty<Real> & _dgammadxplus;
  const ADMaterialProperty<Real> & _dgammadyplus;
  const ADMaterialProperty<Real> & _dgammadzplus;

  const unsigned int _op_num;
  const std::vector<const ADVariableValue *> _vals;

};
