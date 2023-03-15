//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowJoiner.h"
#include "Conversion.h"

registerMooseObject("PorousFlowApp", PorousFlowJoiner);
registerMooseObject("PorousFlowApp", ADPorousFlowJoiner);

template <bool is_ad>
InputParameters
PorousFlowJoinerTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRequiredParam<std::string>("material_property",
                                       "The property that you want joined into a std::vector");
  params.set<std::string>("pf_material_type") = "joiner";
  params.addClassDescription("This Material forms a std::vector of properties, old properties "
                             "(optionally), and derivatives, out of the individual phase "
                             "properties");
  return params;
}

template <bool is_ad>
PorousFlowJoinerTempl<is_ad>::PorousFlowJoinerTempl(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _pf_prop(getParam<std::string>("material_property")),
    _dporepressure_dvar(is_ad              ? nullptr
                        : !_nodal_material ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                 "dPorousFlow_porepressure_qp_dvar")
                                           : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                 "dPorousFlow_porepressure_nodal_dvar")),
    _dsaturation_dvar(is_ad              ? nullptr
                      : !_nodal_material ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_saturation_qp_dvar")
                                         : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_saturation_nodal_dvar")),
    _dtemperature_dvar(
        is_ad ? nullptr
        : !_nodal_material
            ? &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")
            : &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")),
    _has_mass_fraction(!_nodal_material
                           ? hasGenericMaterialProperty<std::vector<std::vector<Real>>, is_ad>(
                                 "PorousFlow_mass_frac_qp")
                           : hasGenericMaterialProperty<std::vector<std::vector<Real>>, is_ad>(
                                 "PorousFlow_mass_frac_nodal")),
    _dmass_fraction_dvar(
        is_ad ? nullptr
        : _has_mass_fraction
            ? (!_nodal_material ? &getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                                      "dPorousFlow_mass_frac_qp_dvar")
                                : &getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                                      "dPorousFlow_mass_frac_nodal_dvar"))
            : nullptr),
    _property(declareGenericProperty<std::vector<Real>, is_ad>(_pf_prop)),
    _dproperty_dvar(
        is_ad ? nullptr
              : &declareProperty<std::vector<std::vector<Real>>>("d" + _pf_prop + "_dvar"))
{
  _phase_property.resize(_num_phases);

  if (!is_ad)
  {
    _dphase_property_dp.resize(_num_phases);
    _dphase_property_ds.resize(_num_phases);
    _dphase_property_dt.resize(_num_phases);
    _dphase_property_dX.resize(_num_phases);
  }

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    const std::string phase = Moose::stringify(ph);
    _phase_property[ph] = &getGenericMaterialProperty<Real, is_ad>(_pf_prop + phase);

    if (!is_ad)
    {
      _dphase_property_dp[ph] =
          &getMaterialPropertyDerivative<Real>(_pf_prop + phase, _pressure_variable_name);
      _dphase_property_ds[ph] =
          &getMaterialPropertyDerivative<Real>(_pf_prop + phase, _saturation_variable_name);
      _dphase_property_dt[ph] =
          &getMaterialPropertyDerivative<Real>(_pf_prop + phase, _temperature_variable_name);
      _dphase_property_dX[ph] =
          &getMaterialPropertyDerivative<Real>(_pf_prop + phase, _mass_fraction_variable_name);
    }
  }
}

template <bool is_ad>
void
PorousFlowJoinerTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp].resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _property[_qp][ph] = (*_phase_property[ph])[_qp];
}

template <bool is_ad>
void
PorousFlowJoinerTempl<is_ad>::computeQpProperties()
{
  initQpStatefulProperties();

  if (!is_ad)
  {
    (*_dproperty_dvar)[_qp].resize(_num_phases);
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      (*_dproperty_dvar)[_qp][ph].resize(_num_var);
      for (unsigned v = 0; v < _num_var; ++v)
      {
        // the "if" conditions in the following are because a nodal_material's derivatives might
        // not have been defined.  If that is the case, then DerivativeMaterial passes back a
        // MaterialProperty with zeroes (for the derivatives), but that property will be sized
        // by the number of quadpoints in the element, which may be smaller than the number of
        // nodes!
        (*_dproperty_dvar)[_qp][ph][v] = 0.0;
        if ((*_dphase_property_dp[ph]).size() > _qp)
          (*_dproperty_dvar)[_qp][ph][v] +=
              (*_dphase_property_dp[ph])[_qp] * (*_dporepressure_dvar)[_qp][ph][v];
        if ((*_dphase_property_ds[ph]).size() > _qp)
          (*_dproperty_dvar)[_qp][ph][v] +=
              (*_dphase_property_ds[ph])[_qp] * (*_dsaturation_dvar)[_qp][ph][v];
        if ((*_dphase_property_dt[ph]).size() > _qp)
          (*_dproperty_dvar)[_qp][ph][v] +=
              (*_dphase_property_dt[ph])[_qp] * (*_dtemperature_dvar)[_qp][v];

        // Only add derivative wrt mass fraction if they exist
        if (_has_mass_fraction)
          if ((*_dphase_property_dX[ph]).size() > _qp)
            (*_dproperty_dvar)[_qp][ph][v] +=
                (*_dphase_property_dX[ph])[_qp] * (*_dmass_fraction_dvar)[_qp][ph][0][v];
      }
    }
  }
}

template class PorousFlowJoinerTempl<false>;
template class PorousFlowJoinerTempl<true>;
