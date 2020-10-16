//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEFVCouplingMaterial.h"

registerMooseObject("MooseTestApp", FEFVCouplingMaterial);

InputParameters
FEFVCouplingMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addCoupledVar("fe_var", 1., "A coupled finite element variable.");
  params.addCoupledVar("fv_var", 1., "A coupled finite volume variable.");
  return params;
}

FEFVCouplingMaterial::FEFVCouplingMaterial(const InputParameters & parameters)
  : Material(parameters),
    _fe_var(adCoupledValue("fe_var")),
    _fv_var(adCoupledValue("fv_var")),
    _fe_prop(isCoupled("fe_var") ? &declareADProperty<Real>("fe_prop") : nullptr),
    _fv_prop(isCoupled("fv_var") ? &declareADProperty<Real>("fv_prop") : nullptr)
{
}

void
FEFVCouplingMaterial::computeQpProperties()
{
  if (_fe_prop)
    (*_fe_prop)[_qp] = _fe_var[_qp];
  if (_fv_prop)
    (*_fv_prop)[_qp] = _fv_var[_qp];
}
