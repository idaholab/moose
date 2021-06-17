//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VarCouplingMaterial.h"

registerMooseObject("MooseTestApp", VarCouplingMaterial);

InputParameters
VarCouplingMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addParam<Real>("base", 0.0, "The baseline of the property");
  params.addParam<Real>("coef", 1.0, "The linear coefficient of the coupled var");
  params.addParam<bool>(
      "declare_old", false, "When True the old value for the material property is declared.");
  params.addParam<TagName>("tag", Moose::SOLUTION_TAG, "The solution vector to be coupled in");
  return params;
}

VarCouplingMaterial::VarCouplingMaterial(const InputParameters & parameters)
  : Material(parameters),
    _var(coupledVectorTagValue("var", "tag")),
    _base(getParam<Real>("base")),
    _coef(getParam<Real>("coef")),
    _diffusion(declareProperty<Real>("diffusion")),
    _diffusion_old(getParam<bool>("declare_old") ? &getMaterialPropertyOld<Real>("diffusion")
                                                 : nullptr)
{
}

void
VarCouplingMaterial::initQpStatefulProperties()
{
  _diffusion[_qp] = _var[_qp];
}

void
VarCouplingMaterial::computeQpProperties()
{
  // If "declare_old" is set, then just use it. The test associated is checking that
  // initQpStatefulProperties can use a coupledValue
  if (_diffusion_old)
    _diffusion[_qp] = (*_diffusion_old)[_qp];
  else
    _diffusion[_qp] = _base + _coef * _var[_qp];
}
