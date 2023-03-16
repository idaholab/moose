//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHParsed.h"

registerMooseObject("PhaseFieldApp", ADSplitCHParsed);

InputParameters
ADSplitCHParsed::validParams()
{
  InputParameters params = ADSplitCHCRes::validParams();
  params.addClassDescription(
      "Split formulation Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  params.addCoupledVar("args", "Vector of additional arguments to F");
  params.deprecateCoupledVar("args", "coupled_variables", "02/27/2024");

  return params;
}

ADSplitCHParsed::ADSplitCHParsed(const InputParameters & parameters)
  : ADSplitCHCRes(parameters),
    _f_name(getParam<MaterialPropertyName>("f_name")),
    _dFdc(getADMaterialProperty<Real>(derivativePropertyNameFirst(_f_name, _var.name())))
{
}

ADReal
ADSplitCHParsed::computeDFDC()
{
  return _dFdc[_qp];
}
