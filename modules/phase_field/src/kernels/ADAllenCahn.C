//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAllenCahn.h"

registerMooseObject("PhaseFieldApp", ADAllenCahn);

InputParameters
ADAllenCahn::validParams()
{
  InputParameters params = ADAllenCahnBase<Real>::validParams();
  params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

ADAllenCahn::ADAllenCahn(const InputParameters & parameters)
  : ADAllenCahnBase<Real>(parameters),
    _f_name(getParam<MaterialPropertyName>("f_name")),
    _dFdEta(getADMaterialProperty<Real>(this->derivativePropertyNameFirst(_f_name, _var.name())))
{
}

ADReal
ADAllenCahn::computeDFDOP()
{
  return _dFdEta[_qp];
}
