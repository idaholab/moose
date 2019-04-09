//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHParsed.h"

registerADMooseObject("PhaseFieldApp", ADSplitCHParsed);

defineADValidParams(
    ADSplitCHParsed,
    ADSplitCHCRes,
    params.addClassDescription(
        "Split formulation Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy");
    params.addRequiredParam<MaterialPropertyName>(
        "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
    params.addCoupledVar("args", "Vector of additional arguments to F"););

template <ComputeStage compute_stage>
ADSplitCHParsed<compute_stage>::ADSplitCHParsed(const InputParameters & parameters)
  : ADSplitCHCRes<compute_stage>(parameters),
    _f_name(adGetParam<MaterialPropertyName>("f_name")),
    _dFdc(adGetADMaterialProperty<Real>(derivativePropertyNameFirst(_f_name, _var.name())))
{
}

template <ComputeStage compute_stage>
ADReal
ADSplitCHParsed<compute_stage>::computeDFDC()
{
  return _dFdc[_qp];
}
