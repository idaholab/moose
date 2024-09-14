//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficient3EqnDittusBoelterFunctorMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp",
                    ADWallHeatTransferCoefficient3EqnDittusBoelterFunctorMaterial);

InputParameters
ADWallHeatTransferCoefficient3EqnDittusBoelterFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addParam<MooseFunctorName>("Hw",
                                    FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                    "Heat transfer coefficient material property");
  params.addRequiredParam<MooseFunctorName>("rho", "Density of the liquid");
  params.addRequiredParam<MooseFunctorName>("vel", "x-component of the liquid velocity");
  params.addRequiredParam<MooseFunctorName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MooseFunctorName>("cp", "Specific heat of the fluid");
  params.addRequiredParam<MooseFunctorName>("mu", "Dynamic viscosity of the fluid");
  params.addRequiredParam<MooseFunctorName>("k", "Heat conductivity of the fluid");
  params.addRequiredParam<MooseFunctorName>("T", "Fluid temperature");
  params.addRequiredParam<MooseFunctorName>("T_wall", "Wall temperature");
  params.addClassDescription(
      "Computes wall heat transfer coefficient using Dittus-Boelter equation");
  return params;
}

ADWallHeatTransferCoefficient3EqnDittusBoelterFunctorMaterial::
    ADWallHeatTransferCoefficient3EqnDittusBoelterFunctorMaterial(
        const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>("rho")),
    _vel(getFunctor<ADReal>("vel")),
    _D_h(getFunctor<ADReal>("D_h")),
    _k(getFunctor<ADReal>("k")),
    _mu(getFunctor<ADReal>("mu")),
    _cp(getFunctor<ADReal>("cp")),
    _T(getFunctor<ADReal>("T")),
    _T_wall(getFunctor<ADReal>("T_wall"))
{
  addFunctorProperty<ADReal>(
      "Hw",
      [this](const auto & r, const auto & t) -> ADReal
      {
        ADReal Pr = THM::Prandtl(_cp(r, t), _mu(r, t), _k(r, t));
        ADReal Re = std::max(1.0, THM::Reynolds(1., _rho(r, t), _vel(r, t), _D_h(r, t), _mu(r, t)));
        ADReal n = (_T(r, t) < _T_wall(r, t)) ? 0.4 : 0.3;
        ADReal Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n);

        return THM::wallHeatTransferCoefficient(Nu, _k(r, t), _D_h(r, t));
      });
}
