//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisoHeatConductionMaterial.h"
#include "Function.h"
#include "ConstantFunction.h"
#include "MooseMesh.h"
#include "RankTwoTensorImplementation.h"

#include "libmesh/quadrature.h"

registerMooseObject("HeatConductionApp", AnisoHeatConductionMaterial);
registerMooseObject("HeatConductionApp", ADAnisoHeatConductionMaterial);

template <bool is_ad>
InputParameters
AnisoHeatConductionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addCoupledVar("temperature", "Coupled variable for temperature.");
  params.addParam<Real>(
      "reference_temperature", 293.0, "Reference temperature for thermal conductivity in Kelvin.");
  params.addParam<std::string>("base_name", "Material property base name.");
  params.addRequiredParam<std::vector<Real>>("thermal_conductivity",
                                             "The thermal conductivity tensor values");
  params.addParam<FunctionName>(
      "thermal_conductivity_temperature_coefficient_function",
      "",
      "Temperature coefficient for thermal conductivity as a function of temperature.");
  params.addRequiredParam<FunctionName>("specific_heat",
                                        "Specific heat as a function of temperature.");
  params.addClassDescription("General-purpose material model for anisotropic heat conduction");
  return params;
}

template <bool is_ad>
AnisoHeatConductionMaterialTempl<is_ad>::AnisoHeatConductionMaterialTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _dim(_subproblem.mesh().dimension()),
    _ref_temp(getParam<Real>("reference_temperature")),

    _has_temp(isCoupled("temperature")),
    _T(coupledGenericValue<is_ad>("temperature")),
    _T_var(coupled("temperature")),
    _T_name(coupledName("temperature", 0)),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),

    _user_provided_thermal_conductivity(getParam<std::vector<Real>>("thermal_conductivity")),
    _thermal_conductivity(
        declareGenericProperty<RankTwoTensor, is_ad>(_base_name + "thermal_conductivity")),
    _dthermal_conductivity_dT(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "thermal_conductivity", _T_name)),
    _thermal_conductivity_temperature_coefficient_function(
        getParam<FunctionName>("thermal_conductivity_temperature_coefficient_function") != ""
            ? &getFunction("thermal_conductivity_temperature_coefficient_function")
            : nullptr),

    _specific_heat(declareGenericProperty<Real, is_ad>(_base_name + "specific_heat")),
    _dspecific_heat_dT(declarePropertyDerivative<Real>(_base_name + "specific_heat", _T_name)),
    _specific_heat_function(&getFunction("specific_heat")),
    _ad_q_point(is_ad ? &_assembly.adQPoints() : nullptr)
{
}

template <bool is_ad>
void
AnisoHeatConductionMaterialTempl<is_ad>::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = _user_provided_thermal_conductivity;
  DerivativeMaterialInterface::initQpStatefulProperties();
}

template <>
auto
AnisoHeatConductionMaterialTempl<true>::genericQPoints()
{
  return (*_ad_q_point)[_qp];
}

template <>
auto
AnisoHeatConductionMaterialTempl<false>::genericQPoints()
{
  return _q_point[_qp];
}

template <bool is_ad>
void
AnisoHeatConductionMaterialTempl<is_ad>::computeQpProperties()
{
  const auto & temp_qp = _T[_qp];
  const auto & p = genericQPoints();

  if (_thermal_conductivity_temperature_coefficient_function)
  {

    _thermal_conductivity[_qp] =
        _user_provided_thermal_conductivity *
        (1.0 + _thermal_conductivity_temperature_coefficient_function->value(temp_qp, p) *
                   (temp_qp - _ref_temp));
    _dthermal_conductivity_dT[_qp] =
        _user_provided_thermal_conductivity *
        _thermal_conductivity_temperature_coefficient_function->timeDerivative(
            MetaPhysicL::raw_value(temp_qp), _q_point[_qp]) *
        MetaPhysicL::raw_value(temp_qp - _ref_temp);
  }
  else
    _thermal_conductivity[_qp] = _user_provided_thermal_conductivity;

  _specific_heat[_qp] = _specific_heat_function->value(temp_qp, p);
  _dspecific_heat_dT[_qp] =
      _specific_heat_function->timeDerivative(MetaPhysicL::raw_value(temp_qp), _q_point[_qp]);
}

template class AnisoHeatConductionMaterialTempl<false>;
template class AnisoHeatConductionMaterialTempl<true>;
