/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialJoiner.h"
#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialJoiner>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredParam<std::string>("material_property", "The property that you want joined into a std::vector.  Old values are not considered.");
  params.addParam<bool>("use_qps", false, "True if property to be joined is at the qps, false if at the nodes. Default is false");
  params.addClassDescription("This Material forms a std::vectors of properties and derivatives (but not Old values) out of the individual phase properties");
  return params;
}

PorousFlowMaterialJoiner::PorousFlowMaterialJoiner(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _use_qps(getParam<bool>("use_qps")),
    _num_phases(_porflow_name_UO.num_phases()),
    _pf_prop(getParam<std::string>("material_property")),

    _dporepressure_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_dvar")),
    _dsaturation_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_dvar")),
    _dtemperature_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_dvar")),
    _dporepressure_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar")),
    _dsaturation_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar")),
    _dtemperature_qp_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_qp_dvar")),

    _property(declareProperty<std::vector<Real> >(_pf_prop)),
    _dproperty_dvar(declareProperty<std::vector<std::vector<Real> > >("d" + _pf_prop + "_dvar"))
{
  _phase_property.resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _phase_property[ph] = &getMaterialProperty<Real>(_pf_prop + Moose::stringify(ph));

  _dphase_property_dp.resize(_num_phases);
  _dphase_property_ds.resize(_num_phases);
  _dphase_property_dt.resize(_num_phases);

  VariableName _pressure_variable_name = "pressure_variable";
  VariableName _saturation_variable_name = "saturation_variable";
  VariableName _temperature_variable_name = "temperature_variable";


  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _dphase_property_dp[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), _pressure_variable_name);
    _dphase_property_ds[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), _saturation_variable_name);
    _dphase_property_dt[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), _temperature_variable_name);
  }
}

void
PorousFlowMaterialJoiner::initQpStatefulProperties()
{
}

void
PorousFlowMaterialJoiner::computeQpProperties()
{
  _property[_qp].resize(_num_phases);
  _dproperty_dvar[_qp].resize(_num_phases);
  const unsigned int num_var = _porflow_name_UO.num_v();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _dproperty_dvar[_qp][ph].resize(num_var);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _property[_qp][ph] = (*_phase_property[ph])[_qp];
    for (unsigned v = 0; v < num_var; ++v)
    {
      if(_use_qps)
      {
        _dproperty_dvar[_qp][ph][v] = (*_dphase_property_dp[ph])[_qp] * _dporepressure_qp_dvar[_qp][ph][v];
        _dproperty_dvar[_qp][ph][v] += (*_dphase_property_ds[ph])[_qp] * _dsaturation_qp_dvar[_qp][ph][v];
        _dproperty_dvar[_qp][ph][v] += (*_dphase_property_dt[ph])[_qp] * _dtemperature_qp_dvar[_qp][ph][v];
      }
      else
      {
        _dproperty_dvar[_qp][ph][v] = (*_dphase_property_dp[ph])[_qp] * _dporepressure_dvar[_qp][ph][v];
        _dproperty_dvar[_qp][ph][v] += (*_dphase_property_ds[ph])[_qp] * _dsaturation_dvar[_qp][ph][v];
        _dproperty_dvar[_qp][ph][v] += (*_dphase_property_dt[ph])[_qp] * _dtemperature_dvar[_qp][ph][v];
      }
    }
  }
}
