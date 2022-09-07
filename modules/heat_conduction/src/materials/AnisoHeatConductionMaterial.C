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
  params.addParam<std::vector<Real>>("thermal_conductivity",
                                     "The thermal conductivity tensor values");
  params.addParam<FunctionName>(
      "thermal_conductivity_temperature_coefficient_function",
      "",
      "Temperature coefficient for thermal conductivity as a function of temperature.");
  params.addParam<FunctionName>("k_11", "", "k_11 component of thermal conductivity tensor.");
  params.addParam<FunctionName>("k_22", "", "k_22 component of thermal conductivity tensor.");
  params.addParam<FunctionName>("k_33", "", "k_33 component of thermal conductivity tensor.");
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
    _T_name(getVar("temperature", 0)->name()),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),

    _user_provided_thermal_conductivity_vector(
        isParamValid("thermal_conductivity")
            ? &getParam<std::vector<GenericReal<is_ad>>>("thermal_conductivity")
            : nullptr),
    _thermal_conductivity(
        declareGenericProperty<RankTwoTensor, is_ad>(_base_name + "thermal_conductivity")),
    _dthermal_conductivity_dT(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "thermal_conductivity", _T_name)),
    _thermal_conductivity_temperature_coefficient_function(
        getParam<FunctionName>("thermal_conductivity_temperature_coefficient_function") != ""
            ? &getFunction("thermal_conductivity_temperature_coefficient_function")
            : nullptr),
    _k_11_function(getParam<FunctionName>("k_11") != "" ? &getFunction("k_11") : nullptr),
    _k_22_function(getParam<FunctionName>("k_22") != "" ? &getFunction("k_22") : nullptr),
    _k_33_function(getParam<FunctionName>("k_33") != "" ? &getFunction("k_33") : nullptr),
    _specific_heat(declareGenericProperty<Real, is_ad>(_base_name + "specific_heat")),
    _dspecific_heat_dT(declarePropertyDerivative<Real>(_base_name + "specific_heat", _T_name)),
    _specific_heat_function(&getFunction("specific_heat")),
    _ad_q_point(is_ad ? &_assembly.adQPoints() : nullptr)
{
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
  GenericRankTwoTensor<is_ad> thermal_conductivity_tensor;
  RankTwoTensor dthermal_conductivity_tensor_dt;
  if (_user_provided_thermal_conductivity_vector && _k_11_function)
    mooseError("Either thermal_conductivity or k_11 parameter should be provided by the user in " +
               _name + ". Both cannot be provided.");

  if (_user_provided_thermal_conductivity_vector)
  {
    thermal_conductivity_tensor =
        GenericRankTwoTensor<is_ad>(*_user_provided_thermal_conductivity_vector);

    if (_thermal_conductivity_temperature_coefficient_function)
    {

      _thermal_conductivity[_qp] =
          thermal_conductivity_tensor *
          (1.0 + _thermal_conductivity_temperature_coefficient_function->value(temp_qp, p) *
                     (temp_qp - _ref_temp));
      _dthermal_conductivity_dT[_qp] =
          MetaPhysicL::raw_value(thermal_conductivity_tensor) *
          _thermal_conductivity_temperature_coefficient_function->timeDerivative(
              MetaPhysicL::raw_value(temp_qp), _q_point[_qp]) *
          MetaPhysicL::raw_value(temp_qp - _ref_temp);
    }
    else
      _thermal_conductivity[_qp] = thermal_conductivity_tensor;
  }
  else if (_k_11_function)
  {
    thermal_conductivity_tensor(0, 0) = _k_11_function->value(temp_qp, p);
    dthermal_conductivity_tensor_dt(0, 0) =
        _k_11_function->timeDerivative(MetaPhysicL::raw_value(temp_qp), _q_point[_qp]);

    if (_k_22_function)
    {
      thermal_conductivity_tensor(1, 1) = _k_22_function->value(temp_qp, p);
      dthermal_conductivity_tensor_dt(1, 1) =
          _k_22_function->timeDerivative(MetaPhysicL::raw_value(temp_qp), _q_point[_qp]);
    }
    else
    {
      thermal_conductivity_tensor(1, 1) = _k_11_function->value(temp_qp, p);
      dthermal_conductivity_tensor_dt(1, 1) = dthermal_conductivity_tensor_dt(0, 0);
    }

    if (_k_33_function)
    {
      thermal_conductivity_tensor(2, 2) = _k_33_function->value(temp_qp, p);
      dthermal_conductivity_tensor_dt(2, 2) =
          _k_33_function->timeDerivative(MetaPhysicL::raw_value(temp_qp), _q_point[_qp]);
    }
    else
    {
      thermal_conductivity_tensor(2, 2) = _k_11_function->value(temp_qp, p);
      dthermal_conductivity_tensor_dt(2, 2) = dthermal_conductivity_tensor_dt(0, 0);
    }

    _thermal_conductivity[_qp] = thermal_conductivity_tensor;
    _dthermal_conductivity_dT[_qp] = dthermal_conductivity_tensor_dt;
  }

  else
    mooseError("Either thermal_conductivity or k_11 parameter must be provided by the user in " +
               _name);

  _specific_heat[_qp] = _specific_heat_function->value(temp_qp, p);
  _dspecific_heat_dT[_qp] =
      _specific_heat_function->timeDerivative(MetaPhysicL::raw_value(temp_qp), _q_point[_qp]);
}

template class AnisoHeatConductionMaterialTempl<false>;
template class AnisoHeatConductionMaterialTempl<true>;
