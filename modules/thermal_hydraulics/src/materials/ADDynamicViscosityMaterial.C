//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDynamicViscosityMaterial.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADDynamicViscosityMaterial);

InputParameters
ADDynamicViscosityMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity property");
  params.addRequiredParam<MaterialPropertyName>("v", "Specific volume property");
  params.addRequiredParam<MaterialPropertyName>("e", "Specific internal energy property");

  params.addRequiredParam<UserObjectName>("fp_1phase", "Single-phase fluid properties");

  params.addClassDescription("Computes dynamic viscosity");

  return params;
}

ADDynamicViscosityMaterial::ADDynamicViscosityMaterial(const InputParameters & parameters)
  : Material(parameters),

    _mu_name(getParam<MaterialPropertyName>("mu")),
    _mu(declareADProperty<Real>(_mu_name)),

    _v(getADMaterialProperty<Real>("v")),

    _e(getADMaterialProperty<Real>("e")),

    _fp_1phase(getUserObject<SinglePhaseFluidProperties>("fp_1phase"))
{
}

void
ADDynamicViscosityMaterial::computeQpProperties()
{
  _mu[_qp] = _fp_1phase.mu_from_v_e(_v[_qp], _e[_qp]);
}
