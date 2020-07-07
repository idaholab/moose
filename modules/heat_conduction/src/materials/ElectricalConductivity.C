//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectricalConductivity.h"
#include "libmesh/quadrature.h"

registerMooseObject("HeatConductionApp", ElectricalConductivity);
registerMooseObject("HeatConductionApp", ADElectricalConductivity);

template <bool is_ad>
InputParameters
ElectricalConductivityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addCoupledVar("temperature", 293.0, "Coupled variable for temperature.");
  params.addParam<std::string>("base_name", "Material property base name.");
  params.addParam<Real>("reference_resistivity",
                        1.68e-8,
                        "Electrical resistivity of the material at reference temperature "
                        "(default is copper resistivity in ohm-m).");
  params.addParam<Real>("temperature_coefficient",
                        0.00386,
                        "Coefficient for calculating dependence of resistivity on temperature "
                        "(with copper as the default material).");
  params.addParam<Real>("reference_temperature",
                        293.0,
                        "Reference temperature for Electrical resistivity in Kelvin.");
  params.addClassDescription("Calculates resistivity and electrical conductivity as a function of "
                             "temperature, using copper for parameter defaults.");
  return params;
}

template <bool is_ad>
ElectricalConductivityTempl<is_ad>::ElectricalConductivityTempl(const InputParameters & parameters)
  : Material(parameters),
    _ref_resis(getParam<Real>("reference_resistivity")),
    _temp_coeff(getParam<Real>("temperature_coefficient")),
    _ref_temp(getParam<Real>("reference_temperature")),
    _has_temp(isCoupled("temperature")),
    _T((_has_temp && !is_ad) ? coupledValue("temperature") : _zero),
    _ad_T((_has_temp && is_ad) ? adCoupledValue("temperature") : _ad_zero),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _electric_conductivity(
        declareGenericProperty<Real, is_ad>(_base_name + "electrical_conductivity")),
    _delectric_conductivity_dT(declareProperty<Real>(_base_name + "electrical_conductivity_dT"))
{
}

template <bool is_ad>
void
ElectricalConductivityTempl<is_ad>::setDerivatives(GenericReal<is_ad> & prop,
                                                   Real dprop_dT,
                                                   const ADReal & ad_T)
{
  if (ad_T < 0)
    prop.derivatives() = 0;
  else
    prop.derivatives() = dprop_dT * ad_T.derivatives();
}

template <>
void
ElectricalConductivityTempl<false>::setDerivatives(Real &, Real, const ADReal &)
{
  mooseError("Mistaken call of setDerivatives in a non-AD ElectricalConductivityTempl version");
}

template <bool is_ad>
void
ElectricalConductivityTempl<is_ad>::computeQpProperties()
{
  Real temp_qp = 0;
  if (is_ad)
  {
    temp_qp = MetaPhysicL::raw_value(_ad_T[_qp]);
  }
  else
  {
    temp_qp = _T[_qp];
  }

  const Real resistivity = _ref_resis * (1.0 + _temp_coeff * (temp_qp - _ref_temp));
  const Real dresistivity_dT = _ref_resis * _temp_coeff;
  _electric_conductivity[_qp] = 1.0 / resistivity;
  _delectric_conductivity_dT[_qp] = -1.0 / (resistivity * resistivity) * dresistivity_dT;

  if (is_ad)
    setDerivatives(_electric_conductivity[_qp], _delectric_conductivity_dT[_qp], _ad_T[_qp]);
}

template class ElectricalConductivityTempl<false>;
template class ElectricalConductivityTempl<true>;
