//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityLinear.h"

registerMooseObject("PorousFlowApp", PorousFlowPorosityLinear);

InputParameters
PorousFlowPorosityLinear::validParams()
{
  InputParameters params = PorousFlowPorosityBase::validParams();
  params.addParam<bool>("strain_at_nearest_qp",
                        false,
                        "When calculating nodal porosity that depends on strain, use the strain at "
                        "the nearest quadpoint.  This adds a small extra computational burden, and "
                        "is not necessary for simulations involving only linear lagrange elements. "
                        " If you set this to true, you will also want to set the same parameter to "
                        "true for related Kernels and Materials");
  params.addRequiredCoupledVar(
      "porosity_ref",
      "The porosity at reference effective porepressure, temperature and volumetric strain.  This "
      "must be a real number or a constant monomial variable (not a linear lagrange or other type "
      "of variable)");
  params.addCoupledVar(
      "P_ref",
      0.0,
      "The reference effective porepressure.  This should usually be a linear-lagrange variable");
  params.addCoupledVar(
      "T_ref",
      0.0,
      "The reference temperature.  This should usually be a linear-lagrange variable");
  params.addCoupledVar("epv_ref",
                       0.0,
                       "The reference volumetric strain.  This must be a real number or a constant "
                       "monomial variable (not a linear lagrange or other type of variable)");
  params.addParam<Real>("P_coeff", 0.0, "Effective porepressure coefficient");
  params.addParam<Real>("T_coeff", 0.0, "Temperature coefficient");
  params.addParam<Real>("epv_coeff", 0.0, "Volumetric-strain coefficient");
  params.addRangeCheckedParam<Real>(
      "porosity_min",
      0.0,
      "porosity_min >= 0",
      "Minimum allowed value of the porosity: if the linear relationship gives "
      "values less than this value, then porosity is set to this value instead");
  params.addParam<Real>(
      "zero_modifier",
      1E-3,
      "If the linear relationship produces porosity < porosity_min, then porosity is set "
      "porosity_min.  This means the derivatives of it will be zero.  However, these zero "
      "derivatives often result in poor NR convergence, so the derivatives are set to "
      "_zero_modifier * (values that are relevant for min porosity) to hint to the NR that "
      "porosity is not always constant");
  params.addParamNamesToGroup("zero_modifier", "Advanced");
  params.addClassDescription(
      "This Material calculates the porosity in PorousFlow simulations using the relationship "
      "porosity_ref + P_coeff * (P - P_ref) + T_coeff * (T - T_ref) + epv_coeff * (epv - epv_ref), "
      "where P is the effective porepressure, T is the temperature and epv is the volumetric "
      "strain");
  return params;
}

PorousFlowPorosityLinear::PorousFlowPorosityLinear(const InputParameters & parameters)
  : PorousFlowPorosityBase(parameters),
    _strain_at_nearest_qp(getParam<bool>("strain_at_nearest_qp")),
    _porosity_min(getParam<Real>("porosity_min")),
    _phi_ref(coupledValue("porosity_ref")),
    _P_ref(_nodal_material ? coupledDofValues("P_ref") : coupledValue("P_ref")),
    _T_ref(_nodal_material ? coupledDofValues("T_ref") : coupledValue("T_ref")),
    _epv_ref(coupledValue("epv_ref")),
    _P_coeff(getParam<Real>("P_coeff")),
    _T_coeff(getParam<Real>("T_coeff")),
    _epv_coeff(getParam<Real>("epv_coeff")),

    _uses_volstrain(parameters.isParamSetByUser("epv_coeff") && _epv_coeff != 0.0),
    _vol_strain_qp(getOptionalMaterialProperty<Real>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_strain_qp_dvar(getOptionalMaterialProperty<std::vector<RealGradient>>(
        "dPorousFlow_total_volumetric_strain_qp_dvar")),

    _uses_pf(parameters.isParamSetByUser("P_coeff") && _P_coeff != 0.0),
    _pf_nodal(getOptionalMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_nodal")),
    _pf_qp(getOptionalMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_qp")),
    _dpf_dvar_nodal(getOptionalMaterialProperty<std::vector<Real>>(
        "dPorousFlow_effective_fluid_pressure_nodal_dvar")),
    _dpf_dvar_qp(getOptionalMaterialProperty<std::vector<Real>>(
        "dPorousFlow_effective_fluid_pressure_qp_dvar")),

    _uses_T(parameters.isParamSetByUser("T_coeff") && _T_coeff != 0.0),
    _temperature_nodal(getOptionalMaterialProperty<Real>("PorousFlow_temperature_nodal")),
    _temperature_qp(getOptionalMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _dtemperature_dvar_nodal(
        getOptionalMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")),
    _dtemperature_dvar_qp(
        getOptionalMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),
    _zero_modifier(getParam<Real>("zero_modifier"))
{
}

void
PorousFlowPorosityLinear::initialSetup()
{
  _pf = _nodal_material ? _pf_nodal.get() : _pf_qp.get();
  _dpf_dvar = _nodal_material ? _dpf_dvar_nodal.get() : _dpf_dvar_qp.get();
  if (!(_pf && _dpf_dvar) && _uses_pf)
  {
    mooseError("PorousFlowPorosityLinear: When P_coeff is given you must have an effective fluid "
               "pressure Material");
  }

  _temperature = _nodal_material ? _temperature_nodal.get() : _temperature_qp.get();
  _dtemperature_dvar =
      _nodal_material ? _dtemperature_dvar_nodal.get() : _dtemperature_dvar_qp.get();
  if (!_temperature && _uses_T)
    mooseError(
        "PorousFlowPorosityLinear: When T_coeff is given you must have a temperature Material");

  if (!(_vol_strain_qp && _dvol_strain_qp_dvar) && _uses_volstrain)
    mooseError("PorousFlowPorosityLinear: When epv_coeff is given you must have a "
               "volumetric-strain Material");

  PorousFlowPorosityBase::initialSetup();
}

void
PorousFlowPorosityLinear::initQpStatefulProperties()
{
  _porosity[_qp] = _phi_ref[0];
  (*_dporosity_dvar)[_qp].assign(_num_var, 0.0);
  (*_dporosity_dgradvar)[_qp].assign(_num_var, 0.0);
}

void
PorousFlowPorosityLinear::computeQpProperties()
{
  // note the [0] below: _phi_ref is a constant monomial and we use [0] regardless of
  // _nodal_material
  Real porosity = _phi_ref[0];
  if (_uses_pf)
    porosity += _P_coeff * ((*_pf)[_qp] - _P_ref[_qp]);
  if (_uses_T)
    porosity += _T_coeff * ((*_temperature)[_qp] - _T_ref[_qp]);
  if (_uses_volstrain)
  {
    // Note that in the following _strain[_qp] is evaluated at q quadpoint
    // So _porosity_nodal[_qp], which should be the nodal value of porosity
    // actually uses the strain at a quadpoint.  This
    // is OK for LINEAR elements, as strain is constant over the element anyway.
    const unsigned qp_to_use =
        (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);
    porosity += _epv_coeff * (_vol_strain_qp[qp_to_use] - _epv_ref[0]);
  }

  if (porosity < _porosity_min)
    _porosity[_qp] = _porosity_min;
  else
    _porosity[_qp] = porosity;

  (*_dporosity_dvar)[_qp].resize(_num_var);
  (*_dporosity_dgradvar)[_qp].resize(_num_var);
  for (unsigned int pvar = 0; pvar < _num_var; ++pvar)
  {
    Real deriv = 0.0;
    if (_uses_pf)
      deriv += _P_coeff * (*_dpf_dvar)[_qp][pvar];
    if (_uses_T)
      deriv += _T_coeff * (*_dtemperature_dvar)[_qp][pvar];
    if (porosity < _porosity_min)
      (*_dporosity_dvar)[_qp][pvar] = _zero_modifier * deriv;
    else
      (*_dporosity_dvar)[_qp][pvar] = deriv;

    RealGradient deriv_grad(0.0, 0.0, 0.0);
    if (_uses_volstrain)
    {
      const unsigned qp_to_use =
          (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);
      deriv_grad += _epv_coeff * _dvol_strain_qp_dvar[qp_to_use][pvar];
    }
    if (porosity < _porosity_min)
      (*_dporosity_dgradvar)[_qp][pvar] = _zero_modifier * deriv_grad;
    else
      (*_dporosity_dgradvar)[_qp][pvar] = deriv_grad;
  }
}
