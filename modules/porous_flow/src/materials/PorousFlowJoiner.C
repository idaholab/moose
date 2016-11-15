/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowJoiner.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowJoiner>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addRequiredParam<std::string>("material_property", "The property that you want joined into a std::vector");
  params.addParam<bool>("at_qps", false, "If true then join quadpoint properties, otherwise join nodal properties");
  params.addParam<bool>("include_old", false, "Join old properties into vectors as well as the current properties");
  params.addClassDescription("This Material forms a std::vector of properties, old properties (optionally), and derivatives, out of the individual phase properties");
  return params;
}

PorousFlowJoiner::PorousFlowJoiner(const InputParameters & parameters) :
    PorousFlowMaterialVectorBase(parameters),

    _pressure_variable_name(_dictator.pressureVariableNameDummy()),
    _saturation_variable_name(_dictator.saturationVariableNameDummy()),
    _temperature_variable_name(_dictator.temperatureVariableNameDummy()),
    _mass_fraction_variable_name(_dictator.massFractionVariableNameDummy()),
    _pf_prop(getParam<std::string>("material_property")),
    _include_old(getParam<bool>("include_old")),
    _at_qps(getParam<bool>("at_qps")),

    _dporepressure_dvar(_at_qps ? getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_qp_dvar") : getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_nodal_dvar")),
    _dsaturation_dvar(_at_qps ? getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_qp_dvar") : getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_nodal_dvar")),
    _dtemperature_dvar(_at_qps ? getMaterialProperty<std::vector<Real> >("dPorousFlow_temperature_qp_dvar") : getMaterialProperty<std::vector<Real> >("dPorousFlow_temperature_nodal_dvar")),

    _property(declareProperty<std::vector<Real> >(_pf_prop)),
    _property_old(_include_old ? &declarePropertyOld<std::vector<Real> >(_pf_prop) : NULL),
    _dproperty_dvar(declareProperty<std::vector<std::vector<Real> > >("d" + _pf_prop + "_dvar"))
{
  _phase_property.resize(_num_phases);
  _dphase_property_dp.resize(_num_phases);
  _dphase_property_ds.resize(_num_phases);
  _dphase_property_dt.resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _phase_property[ph] = &getMaterialProperty<Real>(_pf_prop + Moose::stringify(ph));
    _dphase_property_dp[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), _pressure_variable_name);
    _dphase_property_ds[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), _saturation_variable_name);
    _dphase_property_dt[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), _temperature_variable_name);
  }
}

void
PorousFlowJoiner::initQpStatefulProperties()
{
  _property[_qp].resize(_num_phases);
  _dproperty_dvar[_qp].resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _property[_qp][ph] = (*_phase_property[ph])[_qp];
    _dproperty_dvar[_qp][ph].resize(_num_var);
  }
}

void
PorousFlowJoiner::computeQpProperties()
{
  if (!_include_old)
    initQpStatefulProperties();

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _property[_qp][ph] = (*_phase_property[ph])[_qp];
    for (unsigned v = 0; v < _num_var; ++v)
    {
      _dproperty_dvar[_qp][ph][v] = (*_dphase_property_dp[ph])[_qp] * _dporepressure_dvar[_qp][ph][v];
      _dproperty_dvar[_qp][ph][v] += (*_dphase_property_ds[ph])[_qp] * _dsaturation_dvar[_qp][ph][v];
      _dproperty_dvar[_qp][ph][v] += (*_dphase_property_dt[ph])[_qp] * _dtemperature_dvar[_qp][v];
    }
  }
}
