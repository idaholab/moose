//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallHeatTransferCoefficientWeismanMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "FlowModelSinglePhase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADWallHeatTransferCoefficientWeismanMaterial);

InputParameters
ADWallHeatTransferCoefficientWeismanMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("Hw",
                                        FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL,
                                        "Heat transfer coefficient material property");
  params.addParam<MaterialPropertyName>(
      "rho", FlowModelSinglePhase::DENSITY, "Density of the fluid");
  params.addParam<MaterialPropertyName>("vel", FlowModelSinglePhase::VELOCITY, "Fluid velocity");
  params.addParam<MaterialPropertyName>(
      "D_h", FlowModelSinglePhase::HYDRAULIC_DIAMETER, "Hydraulic diameter");
  params.addParam<MaterialPropertyName>(
      "cp", FlowModelSinglePhase::SPECIFIC_HEAT_CONSTANT_PRESSURE, "Specific heat of the fluid");
  params.addParam<MaterialPropertyName>(
      "mu", FlowModelSinglePhase::DYNAMIC_VISCOSITY, "Dynamic viscosity of the fluid");
  params.addParam<MaterialPropertyName>(
      "k", FlowModelSinglePhase::THERMAL_CONDUCTIVITY, "Heat conductivity of the fluid");
  params.addParam<MaterialPropertyName>(
      "T", FlowModelSinglePhase::TEMPERATURE, "Fluid temperature");
  params.addParam<MaterialPropertyName>("T_wall", FlowModel::TEMPERATURE_WALL, "Wall temperature");
  params.addRequiredParam<Real>(
      "PoD", "The Pitch-to-diameter ratio value being assigned into the property");
  MooseEnum bundle_array("SQUARE TRIANGULAR", "SQUARE");
  params.addParam<MooseEnum>("bundle_array", bundle_array, "The type of the rod bundle array");
  params.addClassDescription(
      "Computes wall heat transfer coefficient for water using the Weisman correlation");
  return params;
}

ADWallHeatTransferCoefficientWeismanMaterial::ADWallHeatTransferCoefficientWeismanMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareADProperty<Real>("Hw")),
    _rho(getADMaterialProperty<Real>("rho")),
    _vel(getADMaterialProperty<Real>("vel")),
    _D_h(getADMaterialProperty<Real>("D_h")),
    _k(getADMaterialProperty<Real>("k")),
    _mu(getADMaterialProperty<Real>("mu")),
    _cp(getADMaterialProperty<Real>("cp")),
    _T(getADMaterialProperty<Real>("T")),
    _T_wall(getADMaterialProperty<Real>("T_wall")),
    _PoD(getParam<Real>("PoD")),
    _bundle_array(getParam<MooseEnum>("bundle_array").getEnum<Bundle_array>())
{
}

void
ADWallHeatTransferCoefficientWeismanMaterial::computeQpProperties()
{
  switch (_bundle_array)
  {
    case Bundle_array::SQUARE:
    {
      if (_PoD > 1.3 || _PoD < 1.1)
        mooseDoOnce(mooseWarning(
            "The Weisman correlation for square arrays is valid when P/D is between 1.1 "
            "and 1.3. Be aware that using values out of this range may lead to "
            "significant errors in your results!"));
      ADReal Pr = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
      ADReal Re = std::max(1.0, THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]));
      ADReal n = (_T[_qp] < _T_wall[_qp]) ? 0.4 : 0.3;
      ADReal Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n) * (1.826 * _PoD - 1.0430);
      _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
      break;
    }
    case Bundle_array::TRIANGULAR:
    {
      if (_PoD > 1.5 || _PoD < 1.1)
        mooseDoOnce(mooseWarning(
            "The Weisman correlation for triangular arrays is valid when P/D is between 1.1 "
            "and 1.5. Be aware that using values out of this range may lead to "
            "significant errors in your results!"));
      ADReal Pr = THM::Prandtl(_cp[_qp], _mu[_qp], _k[_qp]);
      ADReal Re = std::max(1.0, THM::Reynolds(1., _rho[_qp], _vel[_qp], _D_h[_qp], _mu[_qp]));
      ADReal n = (_T[_qp] < _T_wall[_qp]) ? 0.4 : 0.3;
      ADReal Nu = 0.023 * std::pow(Re, 4. / 5.) * std::pow(Pr, n) * (1.130 * _PoD - 0.2609);
      _Hw[_qp] = THM::wallHeatTransferCoefficient(Nu, _k[_qp], _D_h[_qp]);
      break;
    }
    default:
      mooseError("Invalid 'bundle_array' parameter.");
  }
}
