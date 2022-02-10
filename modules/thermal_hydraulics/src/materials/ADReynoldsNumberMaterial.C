//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADReynoldsNumberMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include "MathUtils.h"

registerMooseObject("ThermalHydraulicsApp", ADReynoldsNumberMaterial);

InputParameters
ADReynoldsNumberMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("Re", "Reynolds number property name");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density of the phase");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity of the phase");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity of the phase");

  params.addClassDescription("Computes Reynolds number as a material property");

  return params;
}

ADReynoldsNumberMaterial::ADReynoldsNumberMaterial(const InputParameters & parameters)
  : Material(parameters),

    _Re_name(getParam<MaterialPropertyName>("Re")),

    _rho(getADMaterialProperty<Real>("rho")),

    _vel(getADMaterialProperty<Real>("vel")),

    _D_h(getADMaterialProperty<Real>("D_h")),

    _mu(getADMaterialProperty<Real>("mu")),

    _Re(declareADProperty<Real>(_Re_name))

{
}

void
ADReynoldsNumberMaterial::computeQpProperties()
{
  _Re[_qp] = THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);
}
