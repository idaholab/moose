//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial);

InputParameters
ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial::validParams()
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

ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial::
    ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial(const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareADProperty<Real>("Hw")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _k(getADMaterialProperty<Real>("k")),
    _mu(getADMaterialProperty<Real>("mu")),
    _cp(getADMaterialProperty<Real>("cp")),
    _T(getADMaterialProperty<Real>("T")),
    _T_wall(getADMaterialProperty<Real>("T_wall"))
{
}

void
ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial::computeQpProperties()
{
  ADReal Pr = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
  ADReal Re = std::max(1.0, THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]));
  ADReal n = (_T[_qp] < _T_wall[_qp]) ? 0.4 : 0.3;
  ADReal Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n);

  _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
}
