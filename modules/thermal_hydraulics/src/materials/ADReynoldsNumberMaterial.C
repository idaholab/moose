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
#include "FlowModelSinglePhase.h"
#include "MathUtils.h"

registerMooseObject("ThermalHydraulicsApp", ADReynoldsNumberMaterial);

InputParameters
ADReynoldsNumberMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addParam<MaterialPropertyName>(
      "Re", FlowModelSinglePhase::REYNOLDS_NUMBER, "Reynolds number property name");
  params.addParam<MaterialPropertyName>(
      "rho", FlowModelSinglePhase::DENSITY, "Density of the phase");
  params.addParam<MaterialPropertyName>(
      "vel", FlowModelSinglePhase::VELOCITY, "Velocity of the phase");
  params.addParam<MaterialPropertyName>(
      "D_h", FlowModelSinglePhase::HYDRAULIC_DIAMETER, "Hydraulic diameter");
  params.addParam<MaterialPropertyName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Dynamic viscosity of the phase");

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
