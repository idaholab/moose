//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAllenCahn2.h"

registerADMooseObject("PhaseFieldApp", ADAllenCahn2);

defineADValidParams(
    ADAllenCahn2,
    ADAllenCahnBase,
    params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy");
    params.addRequiredParam<MaterialPropertyName>(
        "f_name",
        "Base name of the free energy function F defined in a DerivativeParsedMaterial");
    params.addParam<bool>("extra_term",0,"If set to true, this adds in another term that occurs when kappa is not considered a constant");
);

template <ComputeStage compute_stage>
ADAllenCahn2<compute_stage>::ADAllenCahn2(const InputParameters & parameters)
  : ADAllenCahnBase<compute_stage, Real>(parameters),
    _f_name(getParam<MaterialPropertyName>("f_name")),
    _dFdEta(getADMaterialProperty<Real>(this->derivativePropertyNameFirst(_f_name, _var.name()))),
    _extra_term(getParam<bool>("extra_term"))
{
}

template <ComputeStage compute_stage>
ADReal
ADAllenCahn2<compute_stage>::computeDFDOP()
{
  ADReal value =  _dFdEta[_qp];
  if(_extra_term == true)
    value *= _grad_u[_qp] * _grad_u[_qp] / 2;
  return value;
}
