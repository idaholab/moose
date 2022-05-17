//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallHeatTransferCoefficient3EqnDittusBoelterMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", WallHeatTransferCoefficient3EqnDittusBoelterMaterial);

InputParameters
WallHeatTransferCoefficient3EqnDittusBoelterMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("Hw",
                                        FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                        "Heat transfer coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density of the liquid");
  params.addRequiredParam<MaterialPropertyName>("vel", "x-component of the liquid velocity");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("cp", "Specific heat of the fluid");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity of the fluid");
  params.addRequiredParam<MaterialPropertyName>("k", "Heat conductivity of the fluid");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("T_wall", "Wall temperature");
  params.addClassDescription(
      "Computes wall heat transfer coefficient using Dittus-Boelter equation");
  return params;
}

WallHeatTransferCoefficient3EqnDittusBoelterMaterial::
    WallHeatTransferCoefficient3EqnDittusBoelterMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareProperty<Real>("Hw")),
    _rho(getMaterialProperty<Real>("rho")),
    _vel(getMaterialProperty<Real>("vel")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _k(getMaterialProperty<Real>("k")),
    _mu(getMaterialProperty<Real>("mu")),
    _cp(getMaterialProperty<Real>("cp")),
    _T(getMaterialProperty<Real>("T")),
    _T_wall(getMaterialProperty<Real>("T_wall"))
{
}

void
WallHeatTransferCoefficient3EqnDittusBoelterMaterial::computeQpProperties()
{
  Real Pr = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
  Real Re = THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]);
  Real n = (_T[_qp] < _T_wall[_qp]) ? 0.4 : 0.3;
  Real Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n);

  _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
}
