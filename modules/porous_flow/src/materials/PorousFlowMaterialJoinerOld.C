/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialJoinerOld.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialJoinerOld>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredParam<std::string>("material_property", "The property that you want joined into a std::vector.  Old values will also be joined into a std::vector");
  params.addClassDescription("This Material forms a std::vector of properties, old properties, and derivatives, out of the individual phase properties");
  return params;
}

PorousFlowMaterialJoinerOld::PorousFlowMaterialJoinerOld(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _num_phases(_porflow_name_UO.num_phases()),
    _pf_prop(getParam<std::string>("material_property")),

    _dporepressure_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_porepressure_dvar")),
    _dsaturation_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_saturation_dvar")),
    _dtemperature_dvar(getMaterialProperty<std::vector<std::vector<Real> > >("dPorousFlow_temperature_dvar")),

    _property(declareProperty<std::vector<Real> >(_pf_prop)),
    _property_old(declarePropertyOld<std::vector<Real> >(_pf_prop)),
    _dproperty_dvar(declareProperty<std::vector<std::vector<Real> > >("d" + _pf_prop + "_dvar"))
{
  _phase_property.resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _phase_property[ph] = &getMaterialProperty<Real>(_pf_prop + Moose::stringify(ph));

    _dphase_property_dp.resize(_num_phases);
    _dphase_property_ds.resize(_num_phases);
    _dphase_property_dt.resize(_num_phases);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _dphase_property_dp[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), "pressure_variable");
    _dphase_property_ds[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), "saturation_variable");
    _dphase_property_dt[ph] = &getMaterialPropertyDerivative<Real>(_pf_prop + Moose::stringify(ph), "temperature_variable");
  }
}

void
PorousFlowMaterialJoinerOld::initQpStatefulProperties()
{
  _property[_qp].resize(_num_phases);
  _property_old[_qp].resize(_num_phases);
  _dproperty_dvar[_qp].resize(_num_phases);
  const unsigned int num_var = _porflow_name_UO.num_v();
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _dproperty_dvar[_qp][ph].resize(num_var);

  //
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _property[_qp][ph] = (*_phase_property[ph])[_qp];
}

void
PorousFlowMaterialJoinerOld::computeQpProperties()
{

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _property[_qp][ph] = (*_phase_property[ph])[_qp];

  const unsigned int num_var = _porflow_name_UO.num_v();
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    for (unsigned v = 0; v < num_var; ++v)
    {
      _dproperty_dvar[_qp][ph][v] = (*_dphase_property_dp[ph])[_qp] * _dporepressure_dvar[_qp][ph][v];
      _dproperty_dvar[_qp][ph][v] += (*_dphase_property_ds[ph])[_qp] * _dsaturation_dvar[_qp][ph][v];
      _dproperty_dvar[_qp][ph][v] += (*_dphase_property_dt[ph])[_qp] * _dtemperature_dvar[_qp][ph][v];
    }
  /*
   *  YAQI HACK !!
   *
   * I really just want to simply set
   * _property[_qp][ph] = (*_phase_property[ph])[_qp];
   * in initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  /*if (_t_step == 1)
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
      _property_old[_qp][ph] = _property[_qp][ph];
  */
}
